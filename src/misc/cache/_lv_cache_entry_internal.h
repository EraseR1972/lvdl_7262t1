/**
* @file _lv_cache_entry_internal.h
*
 */


#ifndef _LV_CACHE_ENTRY_INTERNAL
#define _LV_CACHE_ENTRY_INTERNAL

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_types.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../../osal/lv_os.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void   lv_cache_entry_reset_ref(lv_cache_entry_t * entry);
void   lv_cache_entry_inc_ref(lv_cache_entry_t * entry);
void   lv_cache_entry_dec_ref(lv_cache_entry_t * entry);
void   lv_cache_entry_inc_generation(lv_cache_entry_t * entry);
void   lv_cache_entry_set_generation(lv_cache_entry_t * entry, uint32_t generation);
void   lv_cache_entry_set_node_size(lv_cache_entry_t * entry, uint32_t node_size);
void   lv_cache_entry_set_invalid(lv_cache_entry_t * entry, bool is_invalid);
void   lv_cache_entry_set_cache(lv_cache_entry_t * entry, const lv_cache_t * cache);
void * lv_cache_entry_acquire_data(lv_cache_entry_t * entry);
void   lv_cache_entry_release_data(lv_cache_entry_t * entry, void * user_data);
/*************************
 *    GLOBAL VARIABLES
 *************************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_LV_CACHE_ENTRY_INTERNAL*/
