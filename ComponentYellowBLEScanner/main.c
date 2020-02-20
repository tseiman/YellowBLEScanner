#include "legato.h"
#include "interfaces.h"
#include "DBusBLEScanner.h"

void updateBLEEntry(struct BLE_Scan_s *scan) {
    LE_DEBUG("Updating/adding BLE Address: %s", scan->addr);
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
