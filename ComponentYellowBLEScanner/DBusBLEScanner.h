/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
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

#ifndef HAVE_DBUS_BLE_SCANNER_H
#define HAVE_DBUS_BLE_SCANNER_H 1

/* kind of scan update */
typedef enum {EMPTY, NEW, UPDATE, DELETE} Scan_Update_t;

/* a newly scanned device can be stored in this structure */
struct BLE_Scan_s {
    Scan_Update_t   updateType;
    char            *addr;
    char            *type;
    char            *name;
    gint16          rssi;
};

/* callback function call definitions called on an a bluetooth scan even */
typedef void (*DeviceUpdate_Callback_t)(struct BLE_Scan_s *scan);

/* stop BLE scanning */
void yel_ble_stopScan(void);
/* start scanning */
int yel_ble_startScan(void);
/* setup the scanner (to be called before startScan()) */ 
void yel_ble_setupScan(DeviceUpdate_Callback_t updateCb);

#endif 

/* END */
