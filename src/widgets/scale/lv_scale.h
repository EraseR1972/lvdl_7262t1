/**
 * @file lv_scale.h
 *
 */

#ifndef LV_SCALE_H
#define LV_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"

#if LV_USE_SCALE != 0

#include "../../core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/

/**Default value of total minor ticks. */
#define LV_SCALE_TOTAL_TICK_COUNT_DEFAULT (10U)
LV_EXPORT_CONST_INT(LV_SCALE_TOTAL_TICK_COUNT_DEFAULT);

/**Default value of major tick every nth ticks. */
#define LV_SCALE_MAJOR_TICK_EVERY_DEFAULT (5U)
LV_EXPORT_CONST_INT(LV_SCALE_MAJOR_TICK_EVERY_DEFAULT);

/**Default value of scale label enabled. */
#define LV_SCALE_LABEL_ENABLED_DEFAULT (1U)
LV_EXPORT_CONST_INT(LV_SCALE_LABEL_ENABLED_DEFAULT);

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Scale mode
 */
enum {
    LV_SCALE_MODE_HORIZONTAL_TOP    = 0x00U,
    LV_SCALE_MODE_HORIZONTAL_BOTTOM = 0x01U,
    LV_SCALE_MODE_VERTICAL_LEFT     = 0x02U,
    LV_SCALE_MODE_VERTICAL_RIGHT    = 0x04U,
    LV_SCALE_MODE_ROUND             = 0x08U,
    _LV_SCALE_MODE_LAST
};
typedef uint8_t lv_scale_mode_t;

typedef struct {
    lv_obj_t obj;
    lv_coord_t major_len;
    lv_coord_t minor_len;
    lv_coord_t range_min;
    lv_coord_t range_max;
    uint32_t total_tick_count   : 15;
    uint32_t major_tick_every   : 15;
    lv_scale_mode_t mode;
    uint32_t label_enabled      : 1;
} lv_scale_t;

extern const lv_obj_class_t lv_scale_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an scale object
 * @param parent pointer to an object, it will be the parent of the new scale
 * @return pointer to the created scale
 */
lv_obj_t * lv_scale_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 *=====================*/

/*=====================
 * Setter functions
 *====================*/

/**
 * Set scale mode. See @ref lv_scale_mode_t
 * @param   obj     pointer the scale object
 * @param   mode    New scale mode
 */
void lv_scale_mode(lv_obj_t * obj, lv_scale_mode_t mode);

/**
 * Set scale total tick count (including minor and major ticks)
 * @param   obj       pointer the scale object
 * @param   total_tick_count    New total tick count
 */
void lv_scale_set_total_tick_count(lv_obj_t * obj, lv_coord_t total_tick_count);

/**
 * Sets how often the major tick will be drawn
 * @param   obj       pointer the scale object
 * @param   major_tick_every    New count for major tick drawing
 */
void lv_scale_set_major_tick_every(lv_obj_t * obj, lv_coord_t major_tick_every);

/**
 * Sets label visibility
 * @param   obj       pointer the scale object
 * @param   show_label  Show axis label
 */
void lv_scale_set_label_show(lv_obj_t * obj, bool show_label);

/**
 * Sets major tick length
 * @param   obj       pointer the scale object
 * @param   major_len   Major tick length
 */
void lv_scale_set_major_tick_length(lv_obj_t * obj, lv_coord_t major_len);

/**
 * Sets major tick length
 * @param   obj       pointer the scale object
 * @param   minor_len   Minor tick length
 */
void lv_scale_set_minor_tick_length(lv_obj_t * obj, lv_coord_t minor_len);

/**
 * Set the minimal and maximal values on a scale
 * @param obj       pointer to a scale object
 * @param min       minimum value of the scale
 * @param max       maximum value of the scale
 */
void lv_scale_set_range(lv_obj_t * obj, lv_coord_t min, lv_coord_t max);

/*=====================
 * Getter functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SCALE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCALE_H*/