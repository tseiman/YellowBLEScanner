/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This file contains global usefull macrose.
 * 
 * License: Mozilla Public License 2.0
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */



/* a function call debugging Macro - helpful to understand 
the callstack for BLE  DBUS Scanner */
#ifdef FUNC_DEBUG
#define FUNC_CALL_DEBUG LE_DEBUG("Called %s", __func__)
#else
#define FUNC_CALL_DEBUG 
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* END */
