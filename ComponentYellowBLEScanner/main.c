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
#include "JSONHandler.h"

#include <signal.h>



void processJSONOutput(char *json) {
    LE_INFO("---->>> Would have pushed now: %s", json);
}

void updateBLEEntry(struct BLE_Scan_s *scan) {
    char *buffer = scanToJSON(scan);    
    yel_queue_json_event(buffer);
    free(buffer);
}


static void main_SigHandler(int signal) {
        LE_INFO("Stopping and cleaning up Yellow BLE Scanner");
        yel_ble_stopScan();
        yel_queue_stop();
}


COMPONENT_INIT { 
    
    le_sig_Block(SIGINT);                                                   // catch the termination of the Application
    le_sig_SetEventHandler(SIGINT, main_SigHandler);                        // to clean up allocated resources
    le_sig_Block(SIGTERM);                                                  
    le_sig_SetEventHandler(SIGTERM, main_SigHandler);                      

    LE_INFO("Starting Yellow BLE Scanner");

    yel_queue_init(processJSONOutput);


    yel_ble_setupScan(updateBLEEntry);
    yel_ble_startScan();


}
