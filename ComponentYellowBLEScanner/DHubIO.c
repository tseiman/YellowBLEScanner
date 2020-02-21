/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This module implements the interface to the DHUB API.
 * The scan result and the queue length are send to DHUB
 * an enable/disable switch and a timer setting for the queue
 * can be set through DataHub
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */

#include "legato.h"
#include "interfaces.h"

#include "DHubIO.h"
#include "OutThrottleQueue.h"

static UpdateTimer_Callback_t callbackTimer = NULL;     // the callback in case the timer 
                                                        // setting is updated
static UpdateEnable_Callback_t callbackEnable = NULL;   // the callback in case the 
                                                        // enable/disable (BLE Scan) is updated

/** ------------------------------------------------------------------------
 *
 * This handler is called when enable/disable (BLE Scan) is updated.
 * It calls then the callback to prcess this event elsewhere (in BLE Scanner)
 *
 * @param timestamp     Gives an idication when it was set through DHUB
 * @param value         The boolean setting (enable=true)
 * @param contextPtr    a context pointer (unused)
 *
 * ------------------------------------------------------------------------     
 */
static void ScanEnableUpdateHandler(double timestamp, bool value, void* contextPtr) {
  
    if(callbackEnable) {
        callbackEnable(value);
    } else {
        LE_WARN("No callback for the scan enable set, no action.");
    }
  
}

/** ------------------------------------------------------------------------
 *
 * This handler is called when the timer setting (Queue Timer) is updated.
 * It calls then the callback to prcess this event elsewhere (in ThrottleQueue)
 * 
 * @param timestamp     Gives an idication when it was set through DHUB
 * @param value         The numeric setting in milliseconds
 * @param contextPtr    a context pointer (unused)
 *
 * ------------------------------------------------------------------------     
 */
static void QueueTimerUpdateHandler(double timestamp, double value, void* contextPtr) {
  
    if(callbackTimer) {
        callbackTimer((int) value);
    } else {
        LE_WARN("No callback for the timer setup set, no action.");
    }
  
}

/** ------------------------------------------------------------------------
 *
 * called to update the actual queue length of ThrottleQueue for
 * statistical reasons and to indicate if an overflow/overload
 * of scan events is happening
 * 
 * @param len    the queue length
 *
 * ------------------------------------------------------------------------     
 */
void yel_dhub_pushQueueLength(int len) {
    io_PushNumeric(DHUB_QUEUE_LENGTH, IO_NOW, len);
}

/** ------------------------------------------------------------------------
 *
 * This is called to push the scan event in JSON format. The call is
 * initiated by the queue (when the queue timer triggers the next update)
 * 
 * @param json          the actual scan event in JSON Format
 *
 * ------------------------------------------------------------------------     
 */
void yel_dhub_pushScanEvent(char *json) {
    io_PushJson(DHUB_BLE_SCANEVENT, IO_NOW, json);
}

/** ------------------------------------------------------------------------
 *
 * inititalizes the DHUB API.
 * 
 * @param timerCb          the callback in case the Queue
 *                         timer setting is updated
 * @param enableCb         the callback in case the BLE scanner 
 *                         should be enabled or disabled
 *
 * ------------------------------------------------------------------------     
 */
void yel_dhub_init(UpdateTimer_Callback_t timerCb, UpdateEnable_Callback_t enableCb) {

    le_result_t result;

    callbackEnable = enableCb;
    callbackTimer = timerCb;

    // ================== Outputs (from DHUB to App) ===============
    
    // This is to enable/disable the scanner
    result = io_CreateOutput(DHUB_BLE_SCANENABLE, IO_DATA_TYPE_BOOLEAN, "");
    LE_ASSERT(result == LE_OK);
    io_AddBooleanPushHandler(DHUB_BLE_SCANENABLE, ScanEnableUpdateHandler, NULL);
    io_SetBooleanDefault(DHUB_BLE_SCANENABLE, true);

    // this is to change the queue timer, smaller timer value gives instant results
    // but might overload Octave engine, lager values give delays in scanns results
    // but protects the OCtave engine better
    result = io_CreateOutput(DHUB_QUEUE_TIMER, IO_DATA_TYPE_NUMERIC, "ms");
    LE_ASSERT(result == LE_OK);
    io_AddNumericPushHandler(DHUB_QUEUE_TIMER, QueueTimerUpdateHandler, NULL);
    io_SetNumericDefault(DHUB_QUEUE_TIMER, QUEUE_TIMER_MS_DEFAULT);

    // ================== Inputs (from App to DHUB) ===============

    // this gives some information about the internal queue length
    // it shows if the applications runs into an overlaod situation
    result = io_CreateInput(DHUB_QUEUE_LENGTH, IO_DATA_TYPE_NUMERIC, "count");
    LE_ASSERT(result == LE_OK);

    // this is actually the scan result in JSON format
    //  that is the actual data we're looking for
    result = io_CreateInput(DHUB_BLE_SCANEVENT, IO_DATA_TYPE_JSON, "");
    LE_ASSERT(result == LE_OK);

}
/* END */
