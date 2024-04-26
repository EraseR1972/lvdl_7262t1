/**
 * @file lv_draw_private.h
 *
 */

#ifndef LV_DRAW_PRIVATE_H
#define LV_DRAW_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct lv_draw_task_t {
    lv_draw_task_t * next;

    lv_draw_task_type_t type;

    /**
     * The area where to draw
     */
    lv_area_t area;

    /**
     * The real draw area. E.g. for shadow, outline, or transformed images it's different from `area`
     */
    lv_area_t _real_area;

    /** The original area which is updated*/
    lv_area_t clip_area_original;

    /**
     * The clip area of the layer is saved here when the draw task is created.
     * As the clip area of the layer can be changed as new draw tasks are added its current value needs to be saved.
     * Therefore during drawing the layer's clip area shouldn't be used as it might be already changed for other draw tasks.
     */
    lv_area_t clip_area;

    volatile int state;              /*int instead of lv_draw_task_state_t to be sure its atomic*/

    void * draw_dsc;

    /**
     * The ID of the draw_unit which should take this task
     */
    uint8_t preferred_draw_unit_id;

    /**
     * Set to which extent `preferred_draw_unit_id` is good at this task.
     * 80: means 20% better (faster) than software rendering
     * 100: the default value
     * 110: means 10% worse (slower) than software rendering
     */
    uint8_t preference_score;

};

struct lv_draw_mask_t {
    void * user_data;
};

struct lv_draw_unit_t {
    lv_draw_unit_t * next;

    /**
     * The target_layer on which drawing should happen
     */
    lv_layer_t * target_layer;

    const lv_area_t * clip_area;

    /**
     * Called to try to assign a draw task to itself.
     * `lv_draw_get_next_available_task` can be used to get an independent draw task.
     * A draw task should be assign only if the draw unit can draw it too
     * @param draw_unit     pointer to the draw unit
     * @param layer         pointer to a layer on which the draw task should be drawn
     * @return              >=0:    The number of taken draw task:
     *                                  0 means the task has not yet been completed.
     *                                  1 means a new task has been accepted.
     *                      -1:     The draw unit wanted to work on a task but couldn't do that
     *                              due to some errors (e.g. out of memory).
     *                              It signals that LVGL should call the dispatcher later again
     *                              to let draw unit try to start the rendering again.
     */
    int32_t (*dispatch_cb)(lv_draw_unit_t * draw_unit, lv_layer_t * layer);

    /**
     *
     * @param draw_unit
     * @param task
     * @return
     */
    int32_t (*evaluate_cb)(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);

    /**
     * Called to delete draw unit.
     * @param draw_unit
     * @return
     */
    int32_t (*delete_cb)(lv_draw_unit_t * draw_unit);
};

struct lv_layer_t  {

    /** Target draw buffer of the layer*/
    lv_draw_buf_t * draw_buf;

    /** The absolute coordinates of the buffer */
    lv_area_t buf_area;

    /** The color format of the layer. LV_COLOR_FORMAT_...  */
    lv_color_format_t color_format;

    /**
     * NEVER USE IT DRAW UNITS. USED INTERNALLY DURING DRAW TASK CREATION.
     * The current clip area with absolute coordinates, always the same or smaller than `buf_area`
     * Can be set before new draw tasks are added to indicate the clip area of the draw tasks.
     * Therefore `lv_draw_add_task()` always saves it in the new draw task to know the clip area when the draw task was added.
     * During drawing the draw units also sees the saved clip_area and should use it during drawing.
     * During drawing the layer's clip area shouldn't be used as it might be already changed for other draw tasks.
     */
    lv_area_t _clip_area;

    /** Linked list of draw tasks */
    lv_draw_task_t * draw_task_head;

    lv_layer_t * parent;
    lv_layer_t * next;
    bool all_tasks_added;
    void * user_data;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_PRIVATE_H*/
