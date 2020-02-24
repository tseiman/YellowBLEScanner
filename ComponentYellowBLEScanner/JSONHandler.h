
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
 * License: Mozilla Public License 2.0
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */


#include "DBusBLEScanner.h"

#ifndef HAVE_JSON_HANDLER_H
#define HAVE_JSON_HANDLER_H 1


#ifdef JSON_DEBUG
#define IF_JSON_DEBUG_INFO(fmt, ...) LE_INFO(fmt, ## __VA_ARGS__);
#else
#define IF_JSON_DEBUG_INFO(fmt, ...)
#endif


char *scanToJSON(struct BLE_Scan_s *scan);


#endif

/* END */
