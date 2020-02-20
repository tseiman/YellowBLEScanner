/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reorts it to the Legato DataHub (Octave).
 *
 * This file implements the main routine for the BLE Scanner
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


#include "legato.h"
#include "interfaces.h"
#include "DBusBLEScanner.h"

void updateBLEEntry(struct BLE_Scan_s *scan) {
    LE_DEBUG("Updating/adding BLE Address: %s (%s), with RSSI %d, and name: %s", scan->addr,scan->type, scan->rssi, scan->name);
}

void deleteBLEEntry(char *addr) {
    LE_DEBUG("Deleting BLE Address: %s", addr);
}

COMPONENT_INIT { 
//    le_result_t result;

    LE_INFO("Starting Yellow BLE Scanner");

    yel_ble_setupScan(updateBLEEntry, deleteBLEEntry);
    yel_ble_startScan();


}
