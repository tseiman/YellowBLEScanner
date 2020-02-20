/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reorts it to the Legato DataHub (Octave).
 *
 * This file contains the header definitions for all BLE Scanner structures
 * and functions.
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


#include <glib.h>

#ifndef HAVE_DBUS_BLE_SCANNER
#define HAVE_DBUS_BLE_SCANNER 1

/* a newly scanned device can be stored in this structure */
struct BLE_Scan_s {
    char *addr;
    char *type;
    char *name;
    gint16 rssi;
};

/* callback function call definitions called on an a bluetooth scan even */
typedef void (*DeviceUpdate_Callback_t)(struct BLE_Scan_s *scan);
typedef void (*DeviceDelete_Callback_t)(char *addr);

/* stop BLE scanning */
void yel_ble_stopScan(void);
/* start scanning */
int yel_ble_startScan(void);
/* setup the scanner (to be called before startScan()) */ 
void yel_ble_setupScan(DeviceUpdate_Callback_t updateCb, DeviceDelete_Callback_t deleteCb);

#endif 