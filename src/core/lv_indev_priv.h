/**
 * @file lv_indev_priv.h
 *
 */

#ifndef LV_INDEV_PRIV_H
#define LV_INDEV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_indev_t {
    /**< Input device type*/
    lv_indev_type_t type;

    /**< Function pointer to read input device data.*/
    void (*read_cb)(struct _lv_indev_t * indev, lv_indev_data_t * data);

    /** Called when an action happened on the input device.
     * The second parameter is the event from `lv_event_t`*/
    void (*feedback_cb)(struct _lv_indev_t * indev, uint8_t event_code);

    lv_indev_state_t state; /**< Current state of the input device.*/

    /*Flags*/
    uint8_t long_pr_sent : 1;
    uint8_t reset_query : 1;
    uint8_t disabled : 1;
    uint8_t wait_until_release : 1;

    uint32_t pr_timestamp;         /**< Pressed time stamp*/
    uint32_t longpr_rep_timestamp; /**< Long press repeat time stamp*/

    void * driver_data;

#if LV_USE_USER_DATA
    void * user_data;
#endif

    /**< Pointer to the assigned display*/
    struct _lv_disp_t * disp;

    /**< Timer to periodically read the input device*/
    lv_timer_t * read_timer;

    /**< Number of pixels to slide before actually drag the object*/
    uint8_t scroll_limit;

    /**< Drag throw slow-down in [%]. Greater value means faster slow-down*/
    uint8_t scroll_throw;

    /**< At least this difference should be between two points to evaluate as gesture*/
    uint8_t gesture_min_velocity;

    /**< At least this difference should be to send a gesture*/
    uint8_t gesture_limit;

    /**< Long press time in milliseconds*/
    uint16_t long_press_time;

    /**< Repeated trigger period in long press [ms]*/
    uint16_t long_press_repeat_time;

    struct {
        /*Pointer and button data*/
        lv_point_t act_point; /**< Current point of input device.*/
        lv_point_t last_point; /**< Last point of input device.*/
        lv_point_t last_raw_point; /**< Last point read from read_cb. */
        lv_point_t vect; /**< Difference between `act_point` and `last_point`.*/
        lv_point_t scroll_sum; /*Count the dragged pixels to check LV_INDEV_DEF_SCROLL_LIMIT*/
        lv_point_t scroll_throw_vect;
        lv_point_t scroll_throw_vect_ori;
        struct _lv_obj_t * act_obj;      /*The object being pressed*/
        struct _lv_obj_t * last_obj;     /*The last object which was pressed*/
        struct _lv_obj_t * scroll_obj;   /*The object being scrolled*/
        struct _lv_obj_t * last_pressed; /*The lastly pressed object*/
        lv_area_t scroll_area;
        lv_point_t gesture_sum; /*Count the gesture pixels to check LV_INDEV_DEF_GESTURE_LIMIT*/

        /*Flags*/
        lv_dir_t scroll_dir : 4;
        lv_dir_t gesture_dir : 4;
        uint8_t gesture_sent : 1;
    } pointer;
    struct {
        /*Keypad data*/
        lv_indev_state_t last_state;
        uint32_t last_key;
    } keypad;

    struct _lv_obj_t * cursor;     /**< Cursor for LV_INPUT_TYPE_POINTER*/
    struct _lv_group_t * group;    /**< Keypad destination group*/
    const lv_point_t * btn_points; /**< Array points assigned to the button ()screen will be pressed
                                      here by the buttons*/
} lv_indev_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_INDEV_PRIV_H*/
