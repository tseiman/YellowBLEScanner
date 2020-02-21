
/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This file contains the header definitions for the JSON handling module
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


#include "DBusBLEScanner.h"

#ifndef HAVE_JSON_HANDLER_H
#define HAVE_JSON_HANDLER_H 1

char *scanToJSON(struct BLE_Scan_s *scan);


#endif