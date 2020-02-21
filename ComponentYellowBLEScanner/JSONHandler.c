/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This module implements some functions to handle JSON.
 * 
 * License: Mozilla Public License 2.0
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */

#include "legato.h"
#include "interfaces.h"
#include "DBusBLEScanner.h"
#include <stdio.h>


/** ------------------------------------------------------------------------
 *
 * This function assembles a JSON message out of an Scan struct.
 * CAREFUL - this fucntion allocates memory. It is the _callers_
 * responsibility to free this memory with free().
 *
 * @param scan          the struct with the scan information
 *
 * @return              A pointer to an allocated buffer with the JSON
 *                      string.
 *
 * ------------------------------------------------------------------------     
 */
char *scanToJSON(struct BLE_Scan_s *scan) {
    char *buffer = NULL;

    if(scan->updateType == NEW) {
        asprintf(&buffer,"{ \"type\" : \"new\", \"addr\": \"%s\", \"addrtype\" : \"%s\", \"rssi\": %d, \"name\": \"%s\"}", scan->addr,scan->type, scan->rssi, scan->name);
    } else if (scan->updateType == UPDATE) {
        asprintf(&buffer,"{ \"type\" : \"update\", \"addr\": \"%s\", \"addrtype\" : \"%s\", \"rssi\": %d, \"name\": \"%s\"}", scan->addr,scan->type, scan->rssi, scan->name);
    } else if (scan->updateType == EMPTY) {
        asprintf(&buffer,"{ \"type\" : \"empty\", \"addr\": \"\", \"addrtype\" : \"\", \"rssi\": 0, \"name\": \"\"}");
    } else {
        asprintf(&buffer,"{ \"type\" : \"delete\", \"addr\": \"%s\"}", scan->addr);
    }
 
    return buffer;
}

/* END */
