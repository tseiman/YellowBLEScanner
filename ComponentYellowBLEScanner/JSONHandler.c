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
 * @return              A pointer to an allocated result_buffer with the JSON
 *                      string.
 *
 * ------------------------------------------------------------------------     
 */
char *scanToJSON(struct BLE_Scan_s *scan) {
    char *result_buffer = NULL;
    char *manufacturer_data_buffer = NULL;
    
    if((scan->mdata_size > 0) && (scan->mdata != NULL)) {                   // converting the binary manufacturer data to hex string 
        manufacturer_data_buffer = malloc(scan->mdata_size * 2 +1);
        LE_ASSERT(manufacturer_data_buffer != NULL);
        memset(manufacturer_data_buffer,0,scan->mdata_size * 2 +1);

        for(int i = 0; i < scan->mdata_size; ++i) {
            sprintf(manufacturer_data_buffer + (2 * i), "%02X", scan->mdata[i]);
        }
    }

    if(scan->updateType == NEW) {
        asprintf(&result_buffer,"{ \"type\" : \"new\", \"addr\": \"%s\", \"addrtype\" : \"%s\", \"rssi\": %d, \"name\": \"%s\", \"mdata\": \"%s\"}", scan->addr,scan->type, scan->rssi, scan->name, manufacturer_data_buffer);
    } else if (scan->updateType == UPDATE) {
        asprintf(&result_buffer,"{ \"type\" : \"update\", \"addr\": \"%s\", \"addrtype\" : \"%s\", \"rssi\": %d, \"name\": \"%s\", \"mdata\": \"%s\"}", scan->addr,scan->type, scan->rssi, scan->name, manufacturer_data_buffer);
    } else if (scan->updateType == EMPTY) {
        asprintf(&result_buffer,"{ \"type\" : \"empty\", \"addr\": \"\", \"addrtype\" : \"\", \"rssi\": 0, \"name\": \"\", \"mdata\": \"\"}");
    } else {
        asprintf(&result_buffer,"{ \"type\" : \"delete\", \"addr\": \"%s\"}", scan->addr);
    }
 
    if(manufacturer_data_buffer) free(manufacturer_data_buffer);
    return result_buffer;
}

/* END */
