#include "legato.h"
#include "interfaces.h"

#include <glib.h>

#include "org.bluez.Adapter1.h"
#include "org.bluez.Device1.h"
#include "org.bluez.GattService1.h"
#include "org.bluez.GattCharacteristic1.h"
#include "org.bluez.GattDescriptor1.h"

#include "DBusBLEScanner.h"
#include "global.h"


static GDBusObjectManager *BluezObjectManager = NULL;
static BluezAdapter1 *AdapterInterface = NULL;
static BluezDevice1 *BLEStationDeviceInterface = NULL;

static DeviceUpdate_Callback_t updateCallback = NULL;
static DeviceDelete_Callback_t deleteCallback = NULL;

// static GDBusConnection *con;
static GMainLoop *glibMainLoop;

static enum ApplicationState
{
    APP_STATE_INIT,
    APP_STATE_SEARCHING_FOR_ADAPTER,
    APP_STATE_POWERING_ON_ADAPTER ,
    APP_STATE_SEARCHING_FOR_BLE_STATION //,
  //  APP_STATE_SEARCHING_FOR_ATTRIBUTES,
  //  APP_STATE_SAMPLING
} AppState = APP_STATE_INIT;




static gboolean LegatoFdHandler (GIOChannel *source, GIOCondition condition, gpointer data) {
    while (true) {
        le_result_t r = le_event_ServiceLoop();
        if (r == LE_WOULD_BLOCK) {
            // All of the work is done, so break out
            break;
        }
        LE_ASSERT_OK(r);
    }
    return TRUE;
}


GType BluezProxyTypeFunc(GDBusObjectManagerClient *manager, const gchar *objectPath, const gchar *interfaceName, gpointer userData) {
    FUNC_CALL_DEBUG;
    if (interfaceName == NULL) 
        return g_dbus_object_proxy_get_type();
    

    if (g_strcmp0(interfaceName, "org.bluez.Adapter1") == 0) {
        return BLUEZ_TYPE_ADAPTER1_PROXY;
    } else if (g_strcmp0(interfaceName, "org.bluez.Device1") == 0) {
        return BLUEZ_TYPE_DEVICE1_PROXY;
    } else if (g_strcmp0(interfaceName, "org.bluez.GattService1") == 0) {
        return BLUEZ_TYPE_GATT_SERVICE1_PROXY;
    } else if (g_strcmp0(interfaceName, "org.bluez.GattCharacteristic1") == 0) {
        return BLUEZ_TYPE_GATT_CHARACTERISTIC1_PROXY;
    } else if (g_strcmp0(interfaceName, "org.bluez.GattDescriptor1") == 0) {
        return BLUEZ_TYPE_GATT_DESCRIPTOR1_PROXY;
    }

    return g_dbus_proxy_get_type();
}



static void BeginBLEStationSearch(void) {
    FUNC_CALL_DEBUG;
    AppState = APP_STATE_SEARCHING_FOR_BLE_STATION;
    GError *error = NULL;
    LE_DEBUG("Starting device discovery");
    bluez_adapter1_call_start_discovery_sync(AdapterInterface, NULL, &error);
    LE_FATAL_IF(error != NULL, "Couldn't start discovery - %s", error->message);

    GList *bluezObjects = g_dbus_object_manager_get_objects(BluezObjectManager);
    for (GList *node = bluezObjects; node != NULL && BLEStationDeviceInterface == NULL;  node = node->next) {
        GDBusObject *obj = node->data;

        BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(obj, "org.bluez.Device1"));
        if (dev != NULL)    {
            struct BLE_Scan_s scandata;
            scandata.name = (char *) bluez_device1_get_name(dev);
            scandata.addr = (char *) bluez_device1_get_address(dev);
            scandata.type = (char *) bluez_device1_get_address_type(dev);
            scandata.rssi = bluez_device1_get_rssi(dev);

#ifdef BLE_DEBUG
            LE_DEBUG("-----> found device addr: %s (%s), with RSSI %d, and name: %s", scandata.addr,scandata.type, scandata.rssi, scandata.name);
#endif
            if(updateCallback != NULL) {
                updateCallback(&scandata);
            } else {
                LE_WARN("Callback for BLE scan update is not set. No Action happened.");
            }

        }
    }
    g_list_free_full(bluezObjects, g_object_unref);
}



static void BluezObjectAddedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
    // FUNC_CALL_DEBUG;
#ifdef BLE_DEBUG
    LE_DEBUG("Received \"object-added\" signal - object_path=%s, state=%d", g_dbus_object_get_object_path(object), AppState);
#endif

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL)    {

        struct BLE_Scan_s scandata;

        scandata.name = (char *) bluez_device1_get_name(dev);
        scandata.addr = (char *) bluez_device1_get_address(dev);
        scandata.type = (char *) bluez_device1_get_address_type(dev);
        scandata.rssi = bluez_device1_get_rssi(dev);


#ifdef BLE_DEBUG
        LE_DEBUG("-----> added device addr: %s (%s), with RSSI %d, and name: %s", scandata.addr,scandata.type, scandata.rssi, scandata.name);
#endif

        if(updateCallback != NULL) {
            updateCallback(&scandata);
        } else {
            LE_WARN("Callback for BLE scan update is not set. No Action happened.");
        }
        g_clear_object(&dev);


    }


}


static void BluezObjectRemovedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
    // FUNC_CALL_DEBUG;
#ifdef BLE_DEBUG
    LE_DEBUG("Received \"object-removed\" signal - object_path=%s", g_dbus_object_get_object_path(object));
#endif

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL) {
        char *addr = (char *) bluez_device1_get_address(dev);

        if(deleteCallback != NULL) {
            deleteCallback(addr);
        } else {
            LE_WARN("Callback for BLE scan delete is not set. No Action happened.");
        }
        g_clear_object(&dev);
    }


}

static void AdapterPropertiesChangedHandler( GDBusProxy *proxy, GVariant *changedProperties, GStrv invalidatedProperties, gpointer userData) {
    FUNC_CALL_DEBUG;
    LE_DEBUG("%s - AppState=%d", __func__, AppState);
    if (AppState == APP_STATE_POWERING_ON_ADAPTER)   {
        GVariant *poweredVal =      g_variant_lookup_value(changedProperties, "Powered", G_VARIANT_TYPE_BOOLEAN);
        if (poweredVal != NULL) {
            gboolean powered = g_variant_get_boolean(poweredVal);
            g_variant_unref(poweredVal);
            LE_DEBUG("Adapter Powered property = %d", powered);
            if (powered) {
                BeginBLEStationSearch();
            } 
        }
    }
}

static void AdapterFoundHandler(void){
    FUNC_CALL_DEBUG;
    // Ensure the adapter is powered on
    if (!bluez_adapter1_get_powered(AdapterInterface))
    {
        AppState = APP_STATE_POWERING_ON_ADAPTER;
        LE_DEBUG("Adapter not powered - powering on");
        g_signal_connect(AdapterInterface,"g-properties-changed",G_CALLBACK(AdapterPropertiesChangedHandler), NULL);
        bluez_adapter1_set_powered(AdapterInterface, TRUE);
    } else {
        LE_DEBUG("Adapter already powered");
        BeginBLEStationSearch();
    }
}

static void SearchForAdapter(void) {
    FUNC_CALL_DEBUG;

    AppState = APP_STATE_SEARCHING_FOR_ADAPTER;
    GList *bluezObjects = g_dbus_object_manager_get_objects(BluezObjectManager);
    for (GList *node = bluezObjects; node != NULL && AdapterInterface == NULL; node = node->next) {
        GDBusObject *obj = node->data;
        AdapterInterface = BLUEZ_ADAPTER1(g_dbus_object_get_interface(obj, "org.bluez.Adapter1"));
    }
    g_list_free_full(bluezObjects, g_object_unref);

    if (AdapterInterface != NULL) {
        AdapterFoundHandler();
    }
}


static void BluezObjectManagerCreateCallback(GObject *sourceObject, GAsyncResult *res, gpointer user_data) {
    FUNC_CALL_DEBUG;
    GError *error = NULL;
    BluezObjectManager = g_dbus_object_manager_client_new_for_bus_finish(res, &error);
    LE_FATAL_IF(error != NULL, "Couldn't create Bluez object manager - %s", error->message);

    g_signal_connect(BluezObjectManager, "object-added", G_CALLBACK(BluezObjectAddedHandler), NULL);
    g_signal_connect(BluezObjectManager, "object-removed", G_CALLBACK(BluezObjectRemovedHandler), NULL);

    SearchForAdapter();
}


static void GlibInit(void *deferredArg1, void *deferredArg2) {
//    GMainLoop *glibMainLoop = g_main_loop_new(NULL, FALSE);
    glibMainLoop = g_main_loop_new(NULL, FALSE);

    int legatoEventLoopFd = le_event_GetFd();
    GIOChannel *channel = g_io_channel_unix_new(legatoEventLoopFd);
    gpointer userData = NULL;
    g_io_add_watch(channel, G_IO_IN, LegatoFdHandler, userData);

    g_dbus_object_manager_client_new_for_bus(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
        "org.bluez",
        "/",
        BluezProxyTypeFunc,
        NULL,
        NULL,
        NULL,
        BluezObjectManagerCreateCallback,
        NULL);

    g_main_loop_run(glibMainLoop);

    LE_FATAL("GLib main loop has returned");
}


void yel_ble_stopScan() {
    g_main_loop_quit(glibMainLoop);
}

int yel_ble_startScan() {
    le_event_QueueFunction(GlibInit, NULL, NULL);
    return 0;
}

void yel_ble_setupScan(DeviceUpdate_Callback_t updateCb, DeviceDelete_Callback_t deleteCb) {
    updateCallback = updateCb;
    deleteCallback = deleteCb;
}