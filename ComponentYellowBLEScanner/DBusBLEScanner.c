#include "legato.h"
#include "interfaces.h"

#include <glib.h>

#include "org.bluez.Adapter1.h"
#include "org.bluez.Device1.h"
#include "org.bluez.GattService1.h"
#include "org.bluez.GattCharacteristic1.h"
#include "org.bluez.GattDescriptor1.h"

#include "DBusBLEScanner.h"

static GDBusObjectManager *BluezObjectManager = NULL;
static BluezAdapter1 *AdapterInterface = NULL;
static BluezDevice1 *SensorTagDeviceInterface = NULL;

GDBusConnection *con;

static enum ApplicationState
{
    APP_STATE_INIT,
    APP_STATE_SEARCHING_FOR_ADAPTER,
    APP_STATE_POWERING_ON_ADAPTER,
    APP_STATE_SEARCHING_FOR_SENSORTAG,
    APP_STATE_SEARCHING_FOR_ATTRIBUTES,
    APP_STATE_SAMPLING
} AppState = APP_STATE_INIT;




void yel_ble_stopScan() {

}

int yel_ble_startScan() {

    return 0;
}

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



static void BeginSensorTagSearch(void) {
    LE_DEBUG("------->>>> BeginSensorTagSearch()");
    AppState = APP_STATE_SEARCHING_FOR_SENSORTAG;
    GError *error = NULL;
    LE_DEBUG("Starting device discovery");
    bluez_adapter1_call_start_discovery_sync(AdapterInterface, NULL, &error);
    LE_FATAL_IF(error != NULL, "Couldn't start discovery - %s", error->message);

    GList *bluezObjects = g_dbus_object_manager_get_objects(BluezObjectManager);
    for (GList *node = bluezObjects; node != NULL && SensorTagDeviceInterface == NULL;  node = node->next) {
        GDBusObject *obj = node->data;

  //      SensorTagDeviceInterface = TryCreateSensorTagDeviceProxy(obj);

        BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(obj, "org.bluez.Device1"));
        if (dev != NULL)    {
            const gchar *deviceName = bluez_device1_get_name(dev);
            const gchar *deviceAddr = bluez_device1_get_address(dev);
            const gchar *deviceAddrType = bluez_device1_get_address_type(dev);
            const gint16 deviceRSSI = bluez_device1_get_rssi(dev);

            LE_INFO("-----> found device addr: %s (%s), with RSSI %d, and name: %s", deviceAddr,deviceAddrType, deviceRSSI, deviceName);
        }
/*        if (SensorTagDeviceInterface != NULL)
        {
            SensorTagFoundHandler();
        } */
    }
    g_list_free_full(bluezObjects, g_object_unref);
}


static void AdapterPropertiesChangedHandler( GDBusProxy *proxy, GVariant *changedProperties, GStrv invalidatedProperties, gpointer userData) {
    LE_DEBUG("------->>>> AdapterPropertiesChangedHandler()");
    LE_DEBUG("%s - AppState=%d", __func__, AppState);
    if (AppState == APP_STATE_POWERING_ON_ADAPTER)   {
        GVariant *poweredVal =      g_variant_lookup_value(changedProperties, "Powered", G_VARIANT_TYPE_BOOLEAN);
        if (poweredVal != NULL) {
            gboolean powered = g_variant_get_boolean(poweredVal);
            g_variant_unref(poweredVal);
            LE_DEBUG("Adapter Powered property = %d", powered);
            if (powered) {
                BeginSensorTagSearch();
            } 
        }
    }
}


static void AdapterFoundHandler(void){
         LE_DEBUG("------->>>> AdapterFoundHandler()");
    // Ensure the adapter is powered on
    if (!bluez_adapter1_get_powered(AdapterInterface))
    {
        AppState = APP_STATE_POWERING_ON_ADAPTER;
        LE_DEBUG("Adapter not powered - powering on");
        g_signal_connect(AdapterInterface,"g-properties-changed",G_CALLBACK(AdapterPropertiesChangedHandler), NULL);
        bluez_adapter1_set_powered(AdapterInterface, TRUE);
    } else {
        LE_DEBUG("Adapter already powered");
        BeginSensorTagSearch();
    }
}


static void BluezObjectAddedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
        LE_DEBUG("Received \"object-added\" signal - object_path=%s, state=%d", g_dbus_object_get_object_path(object), AppState);

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL)    {
        const gchar *deviceName = bluez_device1_get_name(dev);
        const gchar *deviceAddr = bluez_device1_get_address
        (dev);
        const gchar *deviceAddrType = bluez_device1_get_address_type(dev);
        const gint16 deviceRSSI = bluez_device1_get_rssi(dev);



        LE_INFO("-----> added device addr: %s (%s), with RSSI %d, and name: %s", deviceAddr,deviceAddrType, deviceRSSI, deviceName);
        g_clear_object(&dev);


    }


}


static void BluezObjectRemovedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
    LE_DEBUG("Received \"object-removed\" signal - object_path=%s", g_dbus_object_get_object_path(object));
}


static void SearchForAdapter(void) {
        LE_DEBUG("------->>>> SearchForAdapter()");

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
    LE_DEBUG("------->>>> BluezObjectManagerCreateCallback()");
    GError *error = NULL;
    BluezObjectManager = g_dbus_object_manager_client_new_for_bus_finish(res, &error);
    LE_FATAL_IF(error != NULL, "Couldn't create Bluez object manager - %s", error->message);

    g_signal_connect(BluezObjectManager, "object-added", G_CALLBACK(BluezObjectAddedHandler), NULL);
    g_signal_connect(BluezObjectManager, "object-removed", G_CALLBACK(BluezObjectRemovedHandler), NULL);

    SearchForAdapter();
}


static void GlibInit(void *deferredArg1, void *deferredArg2) {
    GMainLoop *glibMainLoop = g_main_loop_new(NULL, FALSE);

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

int yel_ble_setupScan() {
    le_event_QueueFunction(GlibInit, NULL, NULL);
    return 0;
}