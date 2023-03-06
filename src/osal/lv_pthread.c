/**
 * @file lv_pthread.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_os.h"
#if LV_USE_PTHREAD

#include <pthread.h>
#include <errno.h>
#include "../misc/lv_log.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int generic_callback(void * user_data);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_res_t lv_thread_init(lv_thread_t * thread, lv_thread_prio_t prio, void (*callback)(void *), size_t stack_size,
                        void * user_data)
{
    LV_UNUSED(prio);
    LV_UNUSED(stack_size);
    thread->callback = callback;
    thread->user_data = user_data;
    pthread_create(thread, NULL, generic_callback, thread);
    return LV_RES_OK;
}

lv_res_t lv_thread_delete(lv_thread_t * thread)
{
    /*How?*/
    return LV_RES_OK;
}

lv_res_t lv_mutex_init(lv_mutex_t * mutex)
{
    int ret = pthread_mutex_init(mutex, NULL);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }
    else {
        return LV_RES_OK;
    }
}

lv_res_t lv_mutex_lock(lv_mutex_t * mutex)
{
    int ret = pthread_mutex_lock(mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }
    else {
        return LV_RES_OK;
    }
}

lv_res_t lv_mutex_lock_isr(lv_mutex_t * mutex)
{
    int ret = pthread_mutex_lock(mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }
    else {
        return LV_RES_OK;
    }
}

lv_res_t lv_mutex_unlock(lv_mutex_t * mutex)
{
    int ret = pthread_mutex_unlock(mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }
    else {
        return LV_RES_OK;
    }
}

lv_res_t lv_mutex_delete(lv_mutex_t * mutex)
{
    pthread_mutex_destroy(mutex);
    return LV_RES_OK;
}

lv_res_t lv_thread_sync_init(lv_thread_sync_t * sync)
{
    int ret = pthread_cond_init(&sync->cond, NULL);
    if(ret) {
        LV_LOG_WARN("Error cond: %d", ret);
        return LV_RES_INV;
    }


    ret = pthread_mutex_init(&sync->mutex, NULL);
    if(ret) {
        LV_LOG_WARN("Error mutex: %d", ret);
        return LV_RES_INV;
    }
    else {
        return LV_RES_OK;
    }

    return LV_RES_OK;
}

lv_res_t lv_thread_sync_wait(lv_thread_sync_t * sync, lv_mutex_t * mutex)
{
    int ret = pthread_mutex_lock(&sync->mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }

    ret = pthread_cond_wait(&sync->cond, &sync->mutex);
    if(ret) {
        LV_LOG_WARN("Error cond wait: %d", ret);
        return LV_RES_INV;
    }


    ret = pthread_mutex_unlock(&sync->mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }

    return LV_RES_OK;
}

lv_res_t lv_thread_sync_signal(lv_thread_sync_t * sync)
{
    int ret = pthread_mutex_lock(&sync->mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }

    ret = pthread_cond_signal(&sync->cond);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }

    ret = pthread_mutex_unlock(&sync->mutex);
    if(ret) {
        LV_LOG_WARN("Error: %d", ret);
        return LV_RES_INV;
    }

    return LV_RES_OK;
}

lv_res_t lv_thread_sync_delete(lv_thread_sync_t * sync)
{
    pthread_cond_destroy(&sync->cond);
    pthread_mutex_destroy(&sync->mutex);
    return LV_RES_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int generic_callback(void * user_data)
{
    lv_thread_t * thread = user_data;
    thread->callback(thread->user_data);
    return 0;
}

#endif /*LV_USE_PTHREAD*/
