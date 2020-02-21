

#ifndef HAVE_OUT_THROTTLE_QUEUE
#define HAVE_OUT_THROTTLE_QUEUE 1


#define MAX_QUEUE_CONTAINER_MEM_POOL_SIZE 128
#define MAX_QUEUE_DATA_MEM_POOL_SIZE 10
#define DEFAULT_LARGE_STRING_POOL_SIZE 1024


#define QUEUE_TIMER_MS 500
typedef struct {
    char *json;
    le_sls_Link_t nextLink;
} QueueContainer_t;

void yel_queue_json_event(char *json);

void yel_queue_init(void);


#endif