
#ifdef FUNC_DEBUG
#define FUNC_CALL_DEBUG LE_DEBUG("Called %s", __func__)
#else
#define FUNC_CALL_DEBUG 
#endif