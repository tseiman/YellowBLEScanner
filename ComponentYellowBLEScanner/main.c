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
#include "DHubIO.h"

#include <signal.h>


static bool enable = true;


/** ------------------------------------------------------------------------
 *
 * Called on an scan event. It calles the JSON module to convert the 
 * scan result from BLE_Scan_s struct into JSON and feeds the result into
 * the Throtteling queue. As well it updates the actual queue length 
 * to DHUB (for statisitcal reasons)
 *
 * @param scan      Struct containing scna results
 *
 * ------------------------------------------------------------------------     
 */
void updateBLEEntry(struct BLE_Scan_s *scan) {
    char *buffer = scanToJSON(scan);    
    yel_queue_json_event(buffer);
    yel_dhub_pushQueueLength(yel_queue_length());
    free(buffer);
}

/** ------------------------------------------------------------------------
 *
 * Called in case the enable/disable setting is updated 
 * trough the DHUB API. It is checking if the setting update is changing
 * anything and controls then the BLE Scanner module
 *
 * @param en        true if the BLE scanner should be enabled, 
 *                  false if it should be disabled
 *
 * ------------------------------------------------------------------------     
 */
void handleEnableEvent(bool en) {
    if(en && (!enable)) {
        LE_INFO("Enable Scanning");
        enable = true;
        yel_ble_startScan();
    } else if ((!en) && enable) {
        LE_INFO("Disable Scanning");
        enable = false;
        yel_ble_stopScan();

    }

}

/** ------------------------------------------------------------------------
 *
 * Signal hander in case the component is quit or killed (no hard kill)
 * It start cleanup in the different modules
 *
 * @param signal        the System Signal (unused)
 *
 * ------------------------------------------------------------------------     
 */
static void main_SigHandler(int signal) {
        LE_INFO("Stopping and cleaning up Yellow BLE Scanner");
        yel_ble_stopScan();
        yel_queue_stop();
}

/** ------------------------------------------------------------------------
 *
 * Initializes this Component
 * It configures:
 *      - A System Signal handler for clean shutdown
 *      - Initializes the queue and provides a callback from DHUB API
 *          where the Queue can feed the scan events to
 *      - It initializes the DHUB API and provides callbacks 
 *          in case the setting (enable, queue timer) updates
 *      - the BLE Scanenr is configured as well with a callback 
 *          which is called in case an scan event happened (local function)
 *      - start the scanning
 *
 * ------------------------------------------------------------------------     
 */
COMPONENT_INIT { 
    struct BLE_Scan_s emptyScandata;
    emptyScandata.updateType = EMPTY; 
  

    LE_INFO("Starting Yellow BLE Scanner");


    le_sig_Block(SIGINT);                                                   // catch the termination of the Application
    le_sig_SetEventHandler(SIGINT, main_SigHandler);                        // to clean up allocated resources
    le_sig_Block(SIGTERM);                                                  
    le_sig_SetEventHandler(SIGTERM, main_SigHandler);                      

    yel_queue_init(yel_dhub_pushScanEvent);

    yel_dhub_init(yel_queue_updateTimer,handleEnableEvent);
    yel_dhub_pushScanEvent(scanToJSON(&emptyScandata));                     // this is a workarround that Octave displays all 
                                                                            // JSON properties in the Resource View
    yel_ble_setupScan(updateBLEEntry);
    yel_ble_startScan();


}


/* END */
