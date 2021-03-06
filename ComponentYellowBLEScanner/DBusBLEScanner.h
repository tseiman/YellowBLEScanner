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
 * License: Mozilla Public License 2.0
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


#include <glib.h>

#ifndef HAVE_DBUS_BLE_SCANNER_H
#define HAVE_DBUS_BLE_SCANNER_H 1


#ifdef BLE_DEBUG
#define IF_BLE_DEBUG_INFO(fmt, ...) LE_INFO(fmt, ## __VA_ARGS__);
#else
#define IF_BLE_DEBUG_INFO(fmt, ...)
#endif

/* 
Type of scan update can be EMPTY, NEW, UPDATE, DELETE 
The typedefinition 
typedef enum {EMPTY, NEW, UPDATE, DELETE} Scan_Update_t;

is subsitituted trough the below macro magic to create
as well a string representation of this ENUM type
for debugging print reasons
*/

#define FOREACH_SCAN_UPDATE_TYPE(SCAN_UPDATE_TYPE) \
        SCAN_UPDATE_TYPE(EMPTY)   \
        SCAN_UPDATE_TYPE(NEW)  \
        SCAN_UPDATE_TYPE(UPDATE)   \
        SCAN_UPDATE_TYPE(DELETE)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_SCAN_UPDATE_TYPE(GENERATE_ENUM)
} Scan_Update_t;

#ifdef BLE_DEBUG
static const char *SCAN_UPDATE_TYPE_STRING[] = {
    FOREACH_SCAN_UPDATE_TYPE(GENERATE_STRING)
};
#endif


/* a newly scanned device can be stored in this structure */
struct BLE_Scan_s {
    Scan_Update_t   updateType;
    char            *addr;
    char            *type;
    char            *name;
    uint8_t         *mdata;
    size_t          mdata_size;
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
