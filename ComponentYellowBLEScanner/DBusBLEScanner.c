/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This file implements the DBUS BlueZ interface to receive BlueZ BLE 
 * Scan events on device updates and device deletions a 
 * separeted callback is called.
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


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


/** ------------------------------------------------------------------------
 *
 * This function is to integrate the glib and the legato event loop
 * Specifies the type of function passed to g_io_add_watch() or 
 * g_io_add_watch_full(), which is called when the requested 
 * condition on a GIOChannel is satisfied.
 * see further documentation:
 * https://developer.gnome.org/glib/stable/glib-IO-Channels.html#GIOFunc
 *
 * @param source        the GIOChannel event source
 * @param condition     the condition which has been satisfied
 * @param data          user data set in g_io_add_watch() or 
 *                      g_io_add_watch_full()
 *
 *
 * @return              the function should return FALSE if the event 
 *                      source should be removed
 *
 * ------------------------------------------------------------------------     
 */
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

/** ------------------------------------------------------------------------
 *
 * A function used to determine the GType to use for an interface proxy 
 * (if interface_name is not NULL) or object proxy (if interface_name 
 * is NULL). This function is called in the thread-default main loop.
 * see further documentation:
 * https://developer.gnome.org/gio/stable/GDBusObjectManagerClient.html#GDBusProxyTypeFunc
 *
 * @param manager           A GDBusObjectManagerClient.
 * @param object_path       The object path of the remote object.
 * @param interface_name    The interface name of the remote object or 
 *                          NULL if a GDBusObjectProxy GType is requested.
 * @param user_data         User data.
 *
 *
 * @return                  A GType to use for the remote object. The 
 *                          returned type must be a GDBusProxy or 
 *                          GDBusObjectProxy -derived type.
 *
 * ------------------------------------------------------------------------     
 */
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


/** ------------------------------------------------------------------------
 *
 * This is called when the scanning starts it will loop trough the 
 * BLE stations which are already known by the BL Adapter and will call
 * for each the update callback
 *
 * ------------------------------------------------------------------------     
 */
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
            scandata.updateType = NEW;
            scandata.name = (char *) bluez_device1_get_name(dev);
            scandata.addr = (char *) bluez_device1_get_address(dev);
            scandata.type = (char *) bluez_device1_get_address_type(dev);
            scandata.rssi = bluez_device1_get_rssi(dev);

#ifdef BLE_DEBUG
            LE_INFO("-----> found device addr: %s (%s), with RSSI %d, and name: %s", scandata.addr,scandata.type, scandata.rssi, scandata.name);
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



/** ------------------------------------------------------------------------
 *
 * This called in case a new BLE station is found. This will call as well
 * the update Callback
 * see further documentation:
 * https://developer.gnome.org/gobject/unstable/gobject-Closures.html#GCallback
 *
 * @param manager       A GDBusObjectManagerClient
 * @param object        a container for a DBUS object. In our case this 
 *                      contains the scanned station
 * @param userData      user data set when the signal handler was connected.
 *
 *
 * ------------------------------------------------------------------------     
 */
static void BluezObjectAddedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
    // FUNC_CALL_DEBUG;
#ifdef BLE_DEBUG
    LE_INFO("Received \"object-added\" signal - object_path=%s, state=%d", g_dbus_object_get_object_path(object), AppState);
#endif

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL)    {

        struct BLE_Scan_s scandata;

        scandata.updateType = NEW;
        scandata.name = (char *) bluez_device1_get_name(dev);
        scandata.addr = (char *) bluez_device1_get_address(dev);
        scandata.type = (char *) bluez_device1_get_address_type(dev);
        scandata.rssi = bluez_device1_get_rssi(dev);


#ifdef BLE_DEBUG
        LE_INFO("-----> added device addr: %s (%s), with RSSI %d, and name: %s", scandata.addr,scandata.type, scandata.rssi, scandata.name);
#endif

        if(updateCallback != NULL) {
            updateCallback(&scandata);
        } else {
            LE_WARN("Callback for BLE scan is not set. No Action happened.");
        }
        g_clear_object(&dev);


    }


}



/** ------------------------------------------------------------------------
 *
 * This called in case a new  station is updated with one or more properties
 * by the adapter. This will call the update Callback.
 * see further documentation:
 * https://developer.gnome.org/gobject/unstable/gobject-Closures.html#GCallback
 *
 * @param TBD
 *
 *
 * ------------------------------------------------------------------------     
 */

static void BluezObjectChangedHandler(GDBusObjectManagerClient *manager,
                                      GDBusObjectProxy *object_proxy,
                                      GDBusProxy *interface_proxy,
                                      GVariant *changed_properties,
                                      const gchar *const *invalidated_properties,
                                      gpointer user_data) {


                                                      
    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(G_DBUS_OBJECT (object_proxy), "org.bluez.Device1"));
    if (dev != NULL)    {

        struct BLE_Scan_s scandata;
        scandata.updateType = UPDATE;
        scandata.name = (char *) bluez_device1_get_name(dev);
        scandata.addr = (char *) bluez_device1_get_address(dev);
        scandata.type = (char *) bluez_device1_get_address_type(dev);
        scandata.rssi = bluez_device1_get_rssi(dev);


#ifdef BLE_DEBUG
        LE_INFO("-----> changed device addr: %s (%s), with RSSI %d, and name: %s", scandata.addr,scandata.type, scandata.rssi, scandata.name);
#endif

        if(updateCallback != NULL) {
            updateCallback(&scandata);
        } else {
            LE_WARN("Callback for BLE scan is not set. No Action happened.");
        }
        g_clear_object(&dev);


    }

}





/** ------------------------------------------------------------------------
 *
 * This called in case a BLE station is removed from the known list
 * by the adapter. This will call the remove Callback.
 * see further documentation:
 * https://developer.gnome.org/gobject/unstable/gobject-Closures.html#GCallback
 *
 * @param manager       A GDBusObjectManagerClient
 * @param object        a container for a DBUS object. In our case this 
 *                      contains the scanned station
 * @param userData      user data set when the signal handler was connected.
 *
 *
 * ------------------------------------------------------------------------     
 */
static void BluezObjectRemovedHandler(GDBusObjectManager *manager, GDBusObject *object, gpointer userData) {
    // FUNC_CALL_DEBUG;
#ifdef BLE_DEBUG
    LE_INFO("Received \"object-removed\" signal - object_path=%s", g_dbus_object_get_object_path(object));
#endif

    BluezDevice1* dev = BLUEZ_DEVICE1(g_dbus_object_get_interface(object, "org.bluez.Device1"));
    if (dev != NULL) {
        struct BLE_Scan_s scandata;
        scandata.updateType = DELETE;
        scandata.name = NULL;
        scandata.addr = (char *) bluez_device1_get_address(dev);
        scandata.type = NULL;
        scandata.rssi = 0;

        
        if(updateCallback != NULL) {
            updateCallback(&scandata);
        } else {
            LE_WARN("Callback for BLE scan is not set. No Action happened.");
        }
        g_clear_object(&dev);
    }


}


/** ------------------------------------------------------------------------
 *
 * This handles a BLE Adapter change notification
 * see further documentation:
 * https://developer.gnome.org/gobject/unstable/gobject-Closures.html#GCallback
 *
 * @param proxy                     GDBusProxy is a base class used for 
 *                                  proxies to access a D-Bus interface 
 *                                  on a remote object
 * @param changedProperties         Map of property name to updated value 
 *                                  for the changed properties.
 * @param invalidatedProperties     Properties whose values have been 
 *                                  invalidated, signature "as".
 * @param userData                  user data set when the 
 *                                  signal handler was connected.
 *
 *
 * ------------------------------------------------------------------------     
 */
static void AdapterPropertiesChangedHandler( GDBusProxy *proxy, GVariant *changedProperties, GStrv invalidatedProperties, gpointer userData) {
    FUNC_CALL_DEBUG;
    LE_INFO("%s - AppState=%d", __func__, AppState);
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

/** ------------------------------------------------------------------------
 *
 * This called once a BL adapter is found. It will try to power the BL 
 * adapter  and initiate the start of the station scan
 *
 * ------------------------------------------------------------------------     
 */
static void AdapterFoundHandler(void){
    FUNC_CALL_DEBUG;
    // Ensure the adapter is powered on
    if (!bluez_adapter1_get_powered(AdapterInterface))
    {
        AppState = APP_STATE_POWERING_ON_ADAPTER;
        LE_INFO("Adapter not powered - powering on");
        g_signal_connect(AdapterInterface,"g-properties-changed",G_CALLBACK(AdapterPropertiesChangedHandler), NULL);
        bluez_adapter1_set_powered(AdapterInterface, TRUE);
    } else {
        LE_INFO("Adapter already powered");
        BeginBLEStationSearch();
    }
}

/** ------------------------------------------------------------------------
 *
 * This is searching an suitable BL adapter running on DBUS
 *
 * ------------------------------------------------------------------------     
 */
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






/** ------------------------------------------------------------------------
 *
 * Here the callbacks are registered for different DBUS signals
 * once that is done searching for a suitable Adapter is initiated
 *
 * ------------------------------------------------------------------------     
 */
static void BluezObjectManagerCreateCallback(GObject *sourceObject, GAsyncResult *res, gpointer user_data) {
    FUNC_CALL_DEBUG;
    GError *error = NULL;
    BluezObjectManager = g_dbus_object_manager_client_new_for_bus_finish(res, &error);
    LE_FATAL_IF(error != NULL, "Couldn't create Bluez object manager - %s", error->message);

    g_signal_connect(BluezObjectManager, "object-added",                        G_CALLBACK(BluezObjectAddedHandler),   NULL);
    g_signal_connect(BluezObjectManager, "object-removed",                      G_CALLBACK(BluezObjectRemovedHandler), NULL);
    g_signal_connect(BluezObjectManager, "interface-proxy-properties-changed",  G_CALLBACK(BluezObjectChangedHandler), NULL);


    SearchForAdapter();
}

/** ------------------------------------------------------------------------
 *
 * This isnitiates the glib event loop
 *
 * ------------------------------------------------------------------------     
 */
static void GlibInit(void *deferredArg1, void *deferredArg2) {
    GError *error = NULL;
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

    LE_WARN("GLib main loop has returned");

    bluez_adapter1_call_stop_discovery_sync(AdapterInterface, NULL, &error);
    LE_WARN_IF(error != NULL, "Couldn't stop discovery - %s", error->message);
}

/** ------------------------------------------------------------------------
 *
 * An interface function to stop the BLE Scan
 *
 * ------------------------------------------------------------------------     
 */
void yel_ble_stopScan() {
    GError *error = NULL;
    bluez_adapter1_call_stop_discovery_sync(AdapterInterface, NULL, &error);
    g_main_loop_quit(glibMainLoop);
}

/** ------------------------------------------------------------------------
 *
 * An interface function to start the BLE Scan
 * the callbacks should be set before with yel_ble_setupScan()
 *
 * ------------------------------------------------------------------------     
 */
int yel_ble_startScan() {
    le_event_QueueFunction(GlibInit, NULL, NULL);
    return 0;
}


/** ------------------------------------------------------------------------
 *
 * An interface function to prepare the BLE scan with callbacks
 *
 * @param updateCb      callback which will be called if a new or 
 *                      updated BLE station is found
 *                               
 * @param deleteCb      callback which is called when the adapter deletes
 *                      stations from the list
 *
 *
 * ------------------------------------------------------------------------     
 */
void yel_ble_setupScan(DeviceUpdate_Callback_t updateCb) {
    updateCallback = updateCb;
}

/* END */