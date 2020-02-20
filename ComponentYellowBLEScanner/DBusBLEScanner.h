

#include <glib.h>

#ifndef HAVE_DBUS_BLE_SCANNER
#define HAVE_DBUS_BLE_SCANNER 1


struct BLE_Scan_s {
    char *addr;
    char *type;
    char *name;
    gint16 rssi;
};

typedef void (*DeviceUpdate_Callback_t)(struct BLE_Scan_s *scan);
typedef void (*DeviceDelete_Callback_t)(char *addr);

void yel_ble_stopScan(void);
int yel_ble_startScan(void);
void yel_ble_setupScan(DeviceUpdate_Callback_t updateCb, DeviceDelete_Callback_t deleteCb);
#endif 