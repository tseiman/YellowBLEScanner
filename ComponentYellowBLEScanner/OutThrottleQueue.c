/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This is part of the YellowBLEScanner Project.
 * YellowBLEScanner is taking BLE scanning data from the mangOH Yellow 
 * BLE Chip and reports it to the Legato DataHub (Octave).
 *
 * This file implements a throtteling mechanism which is meant to prevent 
 * event overflow in the Octave engine in case of a scan flood. 
 * Scan events are wrtten to an event queue and processed over time.
 * This equalizes scan bursts.
 * 
 * License: Mozilla Public License 2.0
 *
 * Project URL: https://github.com/tseiman/YellowBLEScanner  
 *
 ************************************************************************** */

#include "legato.h"
#include "interfaces.h"

#include "OutThrottleQueue.h"

#include <string.h>


/* a string data pool where the buffer for JSON messages is
allocated from. The pointers to the allocated JSON buffers are 
stored in the Linked List containers */
static le_mem_PoolRef_t queueDataPool = NULL;

/* memory pool where Linked List containers are stored */
static le_mem_PoolRef_t queueContainerPool = NULL;

/* the queue linkes list containing all pending queue items */
static le_sls_List_t queueList = LE_SLS_LIST_INIT;

/* the queue timer which initiates the queue processing
to equalize BLE scan bursts*/
static le_timer_Ref_t queueTimer = NULL;

/* the callback called on each queue processing event */
static JSON_Push_Callback_t callback = NULL;

/* define a static memory pool for the JSON strings */
LE_MEM_DEFINE_STATIC_POOL(ConfigStringPool,MAX_QUEUE_DATA_MEM_POOL_SIZE, DEFAULT_LARGE_STRING_POOL_SIZE);

/** ------------------------------------------------------------------------
 *
 * Called every time the queueTimer is kicking. On every timer event one
 * JSON data set is taken from the queue and forwarded
 *
 * ------------------------------------------------------------------------     
 */
void periodicalQueueCheck()  {
    if(! le_sls_IsEmpty(&queueList)) {
        le_sls_Link_t* nextLinkPtr = le_sls_Pop(&queueList);
        QueueContainer_t* containerPtr = CONTAINER_OF(nextLinkPtr, QueueContainer_t, nextLink);

        if (containerPtr != NULL) {
            if(containerPtr->json != NULL) {
#ifdef QUEUE_DEBUG
                LE_INFO("Queue prcessing dataset: %s", containerPtr->json);
#endif
                if(callback != NULL) { 
                    callback(containerPtr->json);
                } else {
                    LE_WARN("No Callback for throttle queue defined. This will end in no action.");
                }
                le_mem_Release(containerPtr->json);
            }
            le_mem_Release(containerPtr);
        }
    }
}


/** ------------------------------------------------------------------------
 *
 * this changes the queue process Timer value. Smaller timer value gives 
 * instant results but might overload Octave engine, lager values give 
 * delays in scanns results but protects the Octave engine better
 *
 * @param msec          the timer value in milliseconds
 *
 * ------------------------------------------------------------------------     
 */
void yel_queue_updateTimer(int msec) {
    int timer_time = msec;
    if(timer_time < QUEUE_TIMER_MS_MIN) timer_time = QUEUE_TIMER_MS_MIN;
    le_timer_SetMsInterval(queueTimer, QUEUE_TIMER_MS_DEFAULT);  
}

/** ------------------------------------------------------------------------
 *
 * this polls the queue length for statisitcal reasons. This can be a help
 * to determine if there is an issue/overload with the queing system
 *
 * @return           the queue length
 *
 * ------------------------------------------------------------------------     
 */
size_t yel_queue_length() {
    return le_sls_NumLinks(&queueList);
}

/** ------------------------------------------------------------------------
 *
 * This is feeding the event queue with JSON data. It is callend on every
 * BLE scan event for each BLE station with the station details in the 
 * JSON dataset
 *
 * @param json          Pointer to the JSON data set. The content of char 
 *                      Buffer is not modified and will not be freed.
 *                      The caller of the function has to clean it's own 
 *                      Buffer if requried.
 *
 * ------------------------------------------------------------------------     
 */
void yel_queue_json_event(char *json) {
    char *jsonBuffer = NULL;
#ifdef QUEUE_DEBUG
    LE_INFO("Got JSON Event: %s", json);
#endif
    QueueContainer_t* containerPtr;
    LE_ASSERT((jsonBuffer = le_mem_StrDup(queueDataPool,json)) != NULL);

    LE_ASSERT((containerPtr = le_mem_ForceAlloc (queueContainerPool)) != NULL);
    containerPtr->nextLink = LE_SLS_LINK_INIT;
    containerPtr->json = jsonBuffer;

    le_sls_Queue(&queueList, &(containerPtr->nextLink));
}

/** ------------------------------------------------------------------------
 *
 * Initializes the Queue, the timer and the memory pools. It needs to be
 * called one time before the queue will be used and every time after a 
 * yel_queue_stop().
 *
 * @param callbk        The callback to be called on each queue processing
 *                      event. (Means on each timer event). This function
 *                      should handle than further JSON handling - e.g.
 *                      pushing it to Legato DataHub.
 *
 * ------------------------------------------------------------------------     
 */
void yel_queue_init(JSON_Push_Callback_t callbk) {

    callback = callbk;
    queueDataPool = le_mem_InitStaticPool(ConfigStringPool, MAX_QUEUE_DATA_MEM_POOL_SIZE, DEFAULT_LARGE_STRING_POOL_SIZE);

    queueContainerPool = le_mem_CreatePool ("ContainerQueuePool", sizeof(QueueContainer_t));
    le_mem_ExpandPool (queueContainerPool, MAX_QUEUE_CONTAINER_MEM_POOL_SIZE);


    queueTimer = le_timer_Create("pushDatafromQueueTimer");       
    le_timer_SetHandler(queueTimer, periodicalQueueCheck); 
    le_timer_SetRepeat(queueTimer, 0);                           
    le_timer_SetMsInterval(queueTimer, QUEUE_TIMER_MS_DEFAULT);  
    le_timer_Start(queueTimer);

}

/** ------------------------------------------------------------------------
 *
 * Stops any queue action and frees the queue memory. For new queue 
 * operation yel_queue_init() because all resources are freed after 
 * yel_queue_stop().
 *
 * ------------------------------------------------------------------------     
 */
void yel_queue_stop() {
    if(queueTimer != NULL) le_timer_Stop(queueTimer);
    
    le_sls_Link_t* nextLinkPtr = le_sls_Pop(&queueList);

    while (nextLinkPtr != NULL) {

        QueueContainer_t* containerPtr = CONTAINER_OF(nextLinkPtr, QueueContainer_t, nextLink);

        if (containerPtr != NULL) {
            if(containerPtr->json != NULL) le_mem_Release(containerPtr->json);
             le_mem_Release(containerPtr);
        }
        nextLinkPtr = le_sls_Pop(&queueList);
    }

} 
/* END */
