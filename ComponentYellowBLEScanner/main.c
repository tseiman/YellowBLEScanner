#include "legato.h"
#include "interfaces.h"
#include "DBusBLEScanner.h"

COMPONENT_INIT { 
//    le_result_t result;

    LE_INFO("Starting Yellow BLE Scanner");

    yel_ble_setupScan();


}
