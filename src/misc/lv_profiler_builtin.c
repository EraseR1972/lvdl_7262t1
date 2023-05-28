/**
 * @file lv_profiler_builtin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_profiler_builtin.h"
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

#if LV_USE_PROFILER_BUILTIN

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Structure representing a built-in profiler item in LVGL
 */
typedef struct {
    char tag;          /**< The tag of the profiler item */
    uint32_t tick;     /**< The tick value of the profiler item */
    const char * func; /**< A pointer to the function associated with the profiler item */
} lv_profiler_builtin_item_t;

/**
 * @brief Structure representing a context for the LVGL built-in profiler
 */
typedef struct {
    lv_profiler_builtin_item_t * item_arr; /**< Pointer to an array of profiler items */
    uint32_t item_num;                     /**< Number of profiler items in the array */
    uint32_t cur_index;                    /**< Index of the current profiler item */
    lv_profiler_builtin_config_t config;   /**< Configuration for the built-in profiler */
} lv_profiler_builtin_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void default_flush_cb(const char * buf);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_profiler_builtin_ctx_t profiler_ctx;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_profiler_builtin_config_init(lv_profiler_builtin_config_t * config)
{
    LV_ASSERT_NULL(config);
    lv_memzero(config, sizeof(lv_profiler_builtin_config_t));
    config->buf_size = LV_PROFILER_BUILTIN_BUF_SIZE;
    config->tick_per_sec = 1000;
    config->tick_get_cb = lv_tick_get;
    config->flush_cb = default_flush_cb;
}

void lv_profiler_builtin_init(const lv_profiler_builtin_config_t * config)
{
    LV_ASSERT_NULL(config);
    uint32_t num = config->buf_size / sizeof(lv_profiler_builtin_item_t);
    if(num == 0) {
        LV_LOG_WARN("buf_size need > %d", (int)sizeof(lv_profiler_builtin_item_t));
        return;
    }

    /*Free the old item_arr memory*/
    if(profiler_ctx.item_arr != NULL) {
        lv_profiler_builtin_uninit();
    }

    lv_memzero(&profiler_ctx, sizeof(profiler_ctx));
    profiler_ctx.item_arr = lv_malloc(num * sizeof(lv_profiler_builtin_item_t));
    LV_ASSERT_MALLOC(profiler_ctx.item_arr);

    if(profiler_ctx.item_arr == NULL) {
        LV_LOG_ERROR("malloc failed for item_arr");
        return;
    }

    profiler_ctx.item_num = num;
    profiler_ctx.config = *config;

    LV_LOG_INFO("init OK, item_num = %d", (int)num);
}

void lv_profiler_builtin_uninit(void)
{
    LV_ASSERT_NULL(profiler_ctx.item_arr);
    lv_free(profiler_ctx.item_arr);
    lv_memzero(&profiler_ctx, sizeof(profiler_ctx));
}

void lv_profiler_builtin_flush(void)
{
    LV_ASSERT_NULL(profiler_ctx.item_arr);
    if(!profiler_ctx.config.flush_cb) {
        LV_LOG_WARN("flush_cb is not registered");
        return;
    }

    uint32_t cur = 0;
    char buf[128];
    double tick_per_sec = profiler_ctx.config.tick_per_sec;
    while(cur < profiler_ctx.cur_index) {
        lv_profiler_builtin_item_t * item = &profiler_ctx.item_arr[cur++];
        lv_snprintf(buf, sizeof(buf),
                    "LVGL-0 [0] %f: tracing_mark_write: %c|0|%s\n",
                    item->tick / tick_per_sec,
                    item->tag,
                    item->func);
        profiler_ctx.config.flush_cb(buf);
    }
}

void lv_profiler_builtin_write(const char * func, char tag)
{
    LV_ASSERT_NULL(profiler_ctx.item_arr);
    LV_ASSERT_NULL(func);
    if(profiler_ctx.cur_index >= profiler_ctx.item_num) {
        lv_profiler_builtin_flush();
        profiler_ctx.cur_index = 0;
    }

    lv_profiler_builtin_item_t * item = &profiler_ctx.item_arr[profiler_ctx.cur_index];
    item->func = func;
    item->tag = tag;
    item->tick = profiler_ctx.config.tick_get_cb();
    profiler_ctx.cur_index++;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void default_flush_cb(const char * buf)
{
    LV_LOG("%s", buf);
}

#endif /*LV_USE_PROFILER_BUILTIN*/
