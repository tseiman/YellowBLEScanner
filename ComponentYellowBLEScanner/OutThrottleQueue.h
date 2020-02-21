/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This header contains defenitions for the ThrottelingQueue
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */

#ifndef HAVE_OUT_THROTTLE_QUEUE_H
#define HAVE_OUT_THROTTLE_QUEUE_H 1


#define MAX_QUEUE_CONTAINER_MEM_POOL_SIZE 128
#define MAX_QUEUE_DATA_MEM_POOL_SIZE 10
#define DEFAULT_LARGE_STRING_POOL_SIZE 1024

#define QUEUE_TIMER_MS_DEFAULT 500
#define QUEUE_TIMER_MS_MIN 1

typedef struct {
    char *json;
    le_sls_Link_t nextLink;
} QueueContainer_t;

typedef void (*JSON_Push_Callback_t)(char *json);

void    yel_queue_json_event(char *json);
void    yel_queue_updateTimer(int msec);
size_t  yel_queue_length(void);
void    yel_queue_init(JSON_Push_Callback_t callbk);
void    yel_queue_stop(void);


#endif

/* END */
