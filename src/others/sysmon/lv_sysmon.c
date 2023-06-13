/**
 * @file lv_sysmon.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_sysmon.h"

#if LV_USE_SYSMON

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_sysmon_class

#define SYSMON_REFR_PERIOD_DEF 300 /* ms */

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_disp_t * disp;
    uint32_t    refr_elaps_sum;
    uint32_t    refr_cnt;
    uint32_t    render_elaps_sum;
    uint32_t    render_cnt;
} perf_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_sysmon_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_sysmon_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void lv_sysmon_timer_cb(lv_timer_t * timer);
static void sysmon_async_cb(void * user_data);

#if LV_USE_PERF_MONITOR
    static void perf_monitor_init(void);
#endif

#if LV_USE_MEM_MONITOR && LV_USE_BUILTIN_MALLOC
    static void mem_monitor_init(void);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_sysmon_class = {
    .base_class = &lv_label_class,
    .constructor_cb = lv_sysmon_constructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .event_cb = lv_sysmon_event,
    .instance_size = sizeof(lv_sysmon_t),
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_sysmon_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_sysmon_set_refr_period(lv_obj_t * obj, uint32_t period)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_sysmon_t * sysmon = (lv_sysmon_t *)obj;
    lv_timer_set_period(sysmon->timer, period);
}

void _lv_sysmon_builtin_init(void)
{
    lv_async_call(sysmon_async_cb, NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_sysmon_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_sysmon_t * sysmon = (lv_sysmon_t *)obj;
    sysmon->timer = lv_timer_create(lv_sysmon_timer_cb, SYSMON_REFR_PERIOD_DEF, obj);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    lv_obj_set_style_pad_all(obj, 3, 0);
    lv_label_set_text(obj, "?");
}

static void lv_sysmon_timer_cb(lv_timer_t * timer)
{
    lv_obj_t * obj = lv_timer_get_user_data(timer);
    lv_obj_send_event(obj, LV_EVENT_REFRESH, NULL);
}

static void lv_sysmon_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);
    lv_obj_event_base(MY_CLASS, e);
}

#if LV_USE_PERF_MONITOR

static void perf_monitor_refr_start_cb(lv_event_t * e)
{
    lv_obj_t * sysmon = lv_event_get_user_data(e);
    perf_info_t * info = lv_obj_get_user_data(sysmon);
    info->refr_elaps_sum += lv_tick_elaps(info->disp->last_render_start_time);
    info->refr_cnt++;
}

static void perf_monitor_refr_finish_cb(lv_event_t * e)
{
    lv_obj_t * sysmon = lv_event_get_user_data(e);
    perf_info_t * info = lv_obj_get_user_data(sysmon);
    info->render_elaps_sum += lv_tick_elaps(info->disp->last_render_start_time);
}

static void perf_monitor_render_start_cb(lv_event_t * e)
{
    lv_obj_t * sysmon = lv_event_get_user_data(e);
    perf_info_t * info = lv_obj_get_user_data(sysmon);
    info->render_cnt++;
}

static void perf_monitor_event_cb(lv_event_t * e)
{
    lv_obj_t * sysmon = lv_event_get_current_target_obj(e);
    perf_info_t * info = lv_obj_get_user_data(sysmon);

    uint32_t cpu = 100 - lv_timer_get_idle();
    uint32_t render_avg_time = info->refr_cnt ? (info->render_elaps_sum / info->refr_cnt) : 0;
    uint32_t fps = info->refr_elaps_sum ? (1000 * info->refr_cnt / info->refr_elaps_sum) : 0;

#if LV_USE_PERF_MONITOR_LOG_MODE
    /*Avoid warning*/
    LV_UNUSED(fps);
    LV_UNUSED(render_avg_time);
    LV_UNUSED(cpu);

    LV_LOG("sysmon: "
           "%" LV_PRIu32" FPS (redraw: %" LV_PRIu32" / refr: %" LV_PRIu32"), "
           "render %" LV_PRIu32" ms, "
           "CPU %" LV_PRIu32 "%%\n",
           fps, info->render_cnt, info->refr_cnt,
           render_avg_time,
           cpu);
#else
    lv_label_set_text_fmt(
        sysmon,
        "%" LV_PRIu32" FPS / %" LV_PRIu32" ms\n%" LV_PRIu32 "%% CPU",
        fps,
        render_avg_time,
        cpu
    );
#endif /*LV_USE_PERF_MONITOR_LOG_MODE*/
    info->refr_elaps_sum = 0;
    info->refr_cnt = 0;
    info->render_elaps_sum = 0;
    info->render_cnt = 0;
}

static void perf_monitor_init(void)
{
    static perf_info_t info = { 0 };
    info.disp = lv_disp_get_default();

    lv_obj_t * sysmon = lv_sysmon_create(lv_layer_sys());
    lv_obj_align(sysmon, LV_USE_PERF_MONITOR_POS, 0, 0);
    lv_obj_set_style_text_align(sysmon, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_user_data(sysmon, &info);
    lv_obj_add_event(sysmon, perf_monitor_event_cb, LV_EVENT_REFRESH, NULL);
    lv_disp_add_event(info.disp, perf_monitor_refr_start_cb, LV_EVENT_REFR_START, sysmon);
    lv_disp_add_event(info.disp, perf_monitor_refr_finish_cb, LV_EVENT_REFR_FINISH, sysmon);
    lv_disp_add_event(info.disp, perf_monitor_render_start_cb, LV_EVENT_RENDER_START, sysmon);

#if LV_USE_PERF_MONITOR_LOG_MODE
    /*Reduce rendering performance consumption*/
    lv_obj_add_flag(sysmon, LV_OBJ_FLAG_HIDDEN);
#endif
}
#endif

#if LV_USE_MEM_MONITOR && LV_USE_BUILTIN_MALLOC
static void mem_monitor_event_cb(lv_event_t * e)
{
    lv_obj_t * sysmon = lv_event_get_current_target_obj(e);
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    uint32_t used_size = mon.total_size - mon.free_size;;
    uint32_t used_kb = used_size / 1024;
    uint32_t used_kb_tenth = (used_size - (used_kb * 1024)) / 102;
    lv_label_set_text_fmt(sysmon,
                          "%"LV_PRIu32 ".%"LV_PRIu32 " kB used (%d %%)\n"
                          "%d%% frag.",
                          used_kb, used_kb_tenth, mon.used_pct,
                          mon.frag_pct);
}

static void mem_monitor_init(void)
{
    lv_obj_t * sysmon = lv_sysmon_create(lv_layer_sys());
    lv_obj_add_event(sysmon, mem_monitor_event_cb, LV_EVENT_REFRESH, NULL);
    lv_obj_align(sysmon, LV_USE_MEM_MONITOR_POS, 0, 0);
}
#endif

static void sysmon_async_cb(void * user_data)
{
    LV_UNUSED(user_data);
#if LV_USE_PERF_MONITOR
    perf_monitor_init();
#endif
#if LV_USE_MEM_MONITOR && LV_USE_BUILTIN_MALLOC
    mem_monitor_init();
#endif
}

#endif /*LV_USE_SYSMON*/
