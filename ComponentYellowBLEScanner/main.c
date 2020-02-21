/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
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
#include "OutThrottleQueue.h"

#include <stdio.h>


void processJSONOutput(char *json) {
    LE_INFO("---->>> Would have pushed now: %s", json);
}

void updateBLEEntry(struct BLE_Scan_s *scan) {
    char *buffer;
    asprintf(&buffer,"{ \"type\" : \"update\", \"addr\": \"%s\", \"addrtype\" : \"%s\", \"rssi\": %d, \"name\": \"%s\"}", scan->addr,scan->type, scan->rssi, scan->name);
    yel_queue_json_event(buffer);
    free(buffer);
//    LE_DEBUG("Updating/adding BLE Address: %s (%s), with RSSI %d, and name: %s", scan->addr,scan->type, scan->rssi, scan->name);
}

void deleteBLEEntry(char *addr) {
    char *buffer;
    asprintf(&buffer,"{ \"type\" : \"delete\", \"addr\": \"%s\"}", addr);
    yel_queue_json_event(buffer);
    free(buffer);
}

COMPONENT_INIT { 
//    le_result_t result;

    LE_INFO("Starting Yellow BLE Scanner");

    yel_queue_init(processJSONOutput);


    yel_ble_setupScan(updateBLEEntry, deleteBLEEntry);
    yel_ble_startScan();


}
