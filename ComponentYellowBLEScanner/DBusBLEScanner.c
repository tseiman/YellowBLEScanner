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

/*

static void AllAttributesFoundHandler() {
    AppState = APP_STATE_SAMPLING;
    GError *error = NULL;

    LE_INFO("All attributes Found");
}
*/
/*
static bool TryProcessAsService( GDBusObject *object, const gchar *objectPath){

    bool found = false;
    BluezGattService1 *serviceProxy = BLUEZ_GATT_SERVICE1(g_dbus_object_get_interface(object, "org.bluez.GattService1"));
    if (serviceProxy == NULL)
        return false;

    const gchar *serviceUuid = bluez_gatt_service1_get_uuid(serviceProxy);
    for (size_t i = 0; i < NUM_ARRAY_MEMBERS(Services); i++) {
        if (g_strcmp0(serviceUuid, Services[i].uuid) == 0) {
            LE_DEBUG("%s service found at: %s", Services[i].name, objectPath);
            LE_ASSERT(Services[i].objectPath == NULL);
            Services[i].objectPath = g_strdup(objectPath);
            found = true;
            break;
        }
    }
    g_clear_object(&serviceProxy);

LE_INFO("TryProcessAsService");
    return found;
} */
/*
static bool TryProcessAsCharacteristic( GDBusObject *object, const gchar *objectPath) {

    BluezGattCharacteristic1 *characteristicProxy = BLUEZ_GATT_CHARACTERISTIC1(g_dbus_object_get_interface(object, "org.bluez.GattCharacteristic1"));
    if (characteristicProxy == NULL)
        return false;
    
    const gchar *characteristicUuid = bluez_gatt_characteristic1_get_uuid(characteristicProxy);

    for (size_t serviceIdx = 0; serviceIdx < NUM_ARRAY_MEMBERS(Services); serviceIdx++) {
        const bool charInService = (
            g_strcmp0( bluez_gatt_characteristic1_get_service(characteristicProxy),
                Services[serviceIdx].objectPath) == 0);
        if (charInService) {
            for (const struct Characteristic *characteristic = Services[serviceIdx].characteristics; characteristic->uuid != NULL; characteristic++) {
                if (g_strcmp0(characteristic->uuid, characteristicUuid) == 0) {
                    LE_ASSERT(*(characteristic->proxy) == NULL);
                    LE_DEBUG("service %s characteristic %s found at %s",Services[serviceIdx].name, characteristic->uuid, objectPath);
                    *(characteristic->proxy) = characteristicProxy;
                    return true;
                }
            }
            break;
        }
    }

    g_clear_object(&characteristicProxy);
 
    LE_INFO("TryProcessAsCharacteristic");
    return false;
}
   */
/*
static void TryProcessAsAttribute( GDBusObject *object, const gchar *devicePath){
     LE_DEBUG("------->>>> TryProcessAsAttribute()");

/ *    gchar *unknownObjectPath = NULL;
    g_object_get(object, "g-object-path", &unknownObjectPath, NULL);
    const bool childOfDevice = g_str_has_prefix(unknownObjectPath, devicePath);
    if (!childOfDevice)  goto done;


    bool missingCharacteristic = false;
    if (TryProcessAsService(object, unknownObjectPath)) {
        // Services are discovered before their attributes, so there's no point in checking if all
        // attributes are found.
    } else if (TryProcessAsCharacteristic(object, unknownObjectPath)) {
        for (size_t serviceIdx = 0; serviceIdx < NUM_ARRAY_MEMBERS(Services) && !missingCharacteristic; serviceIdx++) {
            for (const struct Characteristic *characteristic = Services[serviceIdx].characteristics; characteristic->uuid != NULL; characteristic++) {
                if (*(characteristic->proxy) == NULL) {
                    missingCharacteristic = true;
                    LE_DEBUG("Found missing characteristic: %s:%s", Services[serviceIdx].name, characteristic->uuid);
                    break;
                }
            }
        }

        if (!missingCharacteristic) {
            LE_DEBUG("Found all attributes");
            AllAttributesFoundHandler();
        }
    }

done:
    g_free(unknownObjectPath);
    * /
    LE_INFO("TryProcessAsAttribute");
}
*/
/*
static BluezDevice1 *TryCreateSensorTagDeviceProxy(GDBusObject *object){
 LE_DEBUG("------->>>> TryCreateSensorTagDeviceProxy()");
    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL)    {
        const gchar *deviceName = bluez_device1_get_name(dev);
        const gchar *deviceAddr = bluez_device1_get_address(dev);
        const gchar *deviceAddrType = bluez_device1_get_address_type(dev);
        const gint16 deviceRSSI = bluez_device1_get_rssi(dev);


/ *        if (g_strcmp0("CC2650 SensorTag", deviceName) != 0 && g_strcmp0("CC1350 SensorTag", deviceName) != 0){
            // Not a match, so dispose of the object
            g_clear_object(&dev);
        } * /
LE_INFO("-----> found device addr: %s (%s), with RSSI %d, and name: %s", deviceAddr,deviceAddrType, deviceRSSI, deviceName);
    g_clear_object(&dev);


    }
    return dev;
}
*/

GType BluezProxyTypeFunc(GDBusObjectManagerClient *manager, const gchar *objectPath, const gchar *interfaceName, gpointer userData) {

 //       LE_DEBUG("------->>>> BluezProxyTypeFunc()");

//    LE_DEBUG("proxy ctor: path=%s, intf=%s", objectPath, interfaceName);
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


/*
static void SensorTagFoundHandler(void)
{
        LE_DEBUG("------->>>> SensorTagFoundHandler()");
//    GError *error = NULL;
    LE_DEBUG("SensorTag found - calling Device1.Connect()");
  / *  AppState = APP_STATE_SEARCHING_FOR_ATTRIBUTES;
    bluez_device1_call_connect_sync(SensorTagDeviceInterface, NULL, &error);
    LE_FATAL_IF(error != NULL, "Failed to connect to SensorTag - %s", error->message);

    gchar *devicePath = NULL;
    g_object_get(SensorTagDeviceInterface, "g-object-path", &devicePath, NULL);
    GList *bluezObjects = g_dbus_object_manager_get_objects(BluezObjectManager);
    for (GList *node = bluezObjects;
         node != NULL;
         node = node->next)
    {
        GDBusObject *obj = node->data;
        TryProcessAsAttribute(obj, devicePath);
    }
    g_list_free_full(bluezObjects, g_object_unref);
    g_free(devicePath);* /
}
*/

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
//     LE_DEBUG("------->>>> BluezObjectAddedHandler()");
/*    switch (AppState)
    {
    case APP_STATE_SEARCHING_FOR_ADAPTER:
        AdapterInterface = BLUEZ_ADAPTER1(g_dbus_object_get_interface(object, "org.bluez.Adapter1"));
        if (AdapterInterface != NULL) {
            AdapterFoundHandler();
        }
        break;

    case APP_STATE_SEARCHING_FOR_SENSORTAG:
        {
            SensorTagDeviceInterface = TryCreateSensorTagDeviceProxy(object);
            if (SensorTagDeviceInterface != NULL) {
                SensorTagFoundHandler();
            }
        }
        break;

    case APP_STATE_SEARCHING_FOR_ATTRIBUTES:
        {
            gchar *devicePath = NULL;
            g_object_get(SensorTagDeviceInterface, "g-object-path", &devicePath, NULL);
            TryProcessAsAttribute(object, devicePath);
            g_free(devicePath);
        }
        break;

    default:
        LE_DEBUG("Received \"object-added\" signal - object_path=%s, state=%d", g_dbus_object_get_object_path(object), AppState);
        break;
    }
    */
        LE_DEBUG("Received \"object-added\" signal - object_path=%s, state=%d", g_dbus_object_get_object_path(object), AppState);

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL)    {
        const gchar *deviceName = bluez_device1_get_name(dev);
        const gchar *deviceAddr = bluez_device1_get_address
        (dev);
        const gchar *deviceAddrType = bluez_device1_get_address_type(dev);
        const gint16 deviceRSSI = bluez_device1_get_rssi(dev);


/*        if (g_strcmp0("CC2650 SensorTag", deviceName) != 0 && g_strcmp0("CC1350 SensorTag", deviceName) != 0){
            // Not a match, so dispose of the object
            g_clear_object(&dev);
        } */
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