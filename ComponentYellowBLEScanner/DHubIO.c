
#include "legato.h"
#include "interfaces.h"

#include "DHubIO.h"



/*
#define COUNTER_NAME "counter/value"
#define DATA_NAME "data/value"


static void DataUpdateHandler(double timestamp, double value, void* contextPtr) {
  
    LE_INFO("Received update to 'period' setting: %lf (timestamped %lf)", value, timestamp);
  
}

COMPONENT_INIT { 
    le_result_t result;

    LE_INFO("Starting Yellow BLE Scanner");

    
    // This will be provided to the Data Hub.
    result = io_CreateInput(COUNTER_NAME, IO_DATA_TYPE_NUMERIC, "count");
    LE_ASSERT(result == LE_OK);

    result = io_CreateOutput(DATA_NAME, IO_DATA_TYPE_NUMERIC, "s");
    LE_ASSERT(result == LE_OK);

     io_AddNumericPushHandler(DATA_NAME, DataUpdateHandler, NULL);


}
*/