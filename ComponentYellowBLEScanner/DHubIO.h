/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This module implements the interface to the DHUB API.
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */

#include "legato.h"
#include "interfaces.h"

#ifndef HAVE_DHUB_IO_H
#define HAVE_DHUB_IO_H 1

#define DHUB_BLE_SCANENABLE "bleScanEnable/value"       // BOOL     output  from DHUB to App
#define DHUB_BLE_SCANEVENT  "bleScanEvent/value"        // JSON     input   from App to DHUB << That is where the Scans are going !
#define DHUB_QUEUE_LENGTH   "queueLength/value"         // NUMERIC  input   from App to DHUB
#define DHUB_QUEUE_TIMER    "queueTimer/value"          // NUMERIC  output  from DHUB to App


#define DHUB_QUEUE_TIMER_DEFUALT    500

typedef void (*UpdateTimer_Callback_t)(int msec);
typedef void (*UpdateEnable_Callback_t)(bool enable);


void yel_dhub_pushQueueLength(int len);
void yel_dhub_pushScanEvent(char *json);
void yel_dhub_init(UpdateTimer_Callback_t timerCb, UpdateEnable_Callback_t enableCb);


#endif

/* END */
