#include "legato.h"
#include "interfaces.h"

#include "OutThrottleQueue.h"

#include <string.h>


static le_mem_PoolRef_t queueDataPool = NULL;
static le_mem_PoolRef_t queueContainerPool = NULL;
static le_sls_List_t queueList = LE_SLS_LIST_INIT;
static le_timer_Ref_t queueTimer = NULL;


LE_MEM_DEFINE_STATIC_POOL(ConfigStringPool,MAX_QUEUE_DATA_MEM_POOL_SIZE, DEFAULT_LARGE_STRING_POOL_SIZE);

void periodicalQueueCheck()  {
    if(! le_sls_IsEmpty(&queueList)) {
        le_sls_Link_t* nextLinkPtr = le_sls_Pop(&queueList);
        QueueContainer_t* containerPtr = CONTAINER_OF(nextLinkPtr, QueueContainer_t, nextLink);

        if (containerPtr != NULL) {
            if(containerPtr->json != NULL) {
                LE_INFO("---->>> Would have pushed now: %s", containerPtr->json);
                le_mem_Release(containerPtr->json);
            }
            le_mem_Release(containerPtr);
        }
    }
}

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


void yel_queue_init() {

    queueDataPool = le_mem_InitStaticPool(ConfigStringPool, MAX_QUEUE_DATA_MEM_POOL_SIZE, DEFAULT_LARGE_STRING_POOL_SIZE);

    queueContainerPool = le_mem_CreatePool ("ContainerQueuePool", sizeof(QueueContainer_t));
    le_mem_ExpandPool (queueContainerPool, MAX_QUEUE_CONTAINER_MEM_POOL_SIZE);


    queueTimer = le_timer_Create("pushDatafromQueueTimer");       
    le_timer_SetHandler(queueTimer, periodicalQueueCheck); 
    le_timer_SetRepeat(queueTimer, 0);                           
    le_timer_SetMsInterval(queueTimer, QUEUE_TIMER_MS);  
    le_timer_Start(queueTimer);

}

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
