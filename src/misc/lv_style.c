/**
 * @file lv_style.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_style.h"
#include "../misc/lv_mem.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  GLOBAL VARIABLES
 **********************/

const uint8_t lv_style_builtin_prop_flag_lookup_table[_LV_STYLE_NUM_BUILT_IN_PROPS] = {
    [LV_STYLE_WIDTH] =                    LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_MIN_WIDTH] =                LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_MAX_WIDTH] =                LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_HEIGHT] =                   LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_MIN_HEIGHT] =               LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_MAX_HEIGHT] =               LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_X] =                        LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_Y] =                        LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_ALIGN] =                    LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_TRANSFORM_WIDTH] =           LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_TRANSFORM_HEIGHT] =          LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_TRANSLATE_X] =               LV_STYLE_PROP_LAYOUT_REFR | LV_STYLE_PROP_PARENT_LAYOUT_REFR,
    [LV_STYLE_TRANSLATE_Y] =               LV_STYLE_PROP_LAYOUT_REFR | LV_STYLE_PROP_PARENT_LAYOUT_REFR,
    [LV_STYLE_TRANSFORM_ZOOM] =            LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR | LV_STYLE_PROP_PARENT_LAYOUT_REFR,
    [LV_STYLE_TRANSFORM_ANGLE] =           LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR | LV_STYLE_PROP_PARENT_LAYOUT_REFR,

    [LV_STYLE_PAD_TOP] =                   LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_PAD_BOTTOM] =                LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_PAD_LEFT] =                  LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_PAD_RIGHT] =                 LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_PAD_ROW] =                   LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_PAD_COLUMN] =                LV_STYLE_PROP_EXT_DRAW | LV_STYLE_PROP_LAYOUT_REFR,

    [LV_STYLE_BG_COLOR] = 0,
    [LV_STYLE_BG_OPA] = 0,
    [LV_STYLE_BG_GRAD_COLOR] = 0,
    [LV_STYLE_BG_GRAD_DIR] = 0,
    [LV_STYLE_BG_MAIN_STOP] = 0,
    [LV_STYLE_BG_GRAD_STOP] = 0,
    [LV_STYLE_BG_GRAD] = 0,
    [LV_STYLE_BG_DITHER_MODE] = 0,

    [LV_STYLE_BG_IMG_SRC] =                LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_BG_IMG_OPA] = 0,
    [LV_STYLE_BG_IMG_RECOLOR] = 0,
    [LV_STYLE_BG_IMG_RECOLOR_OPA] = 0,
    [LV_STYLE_BG_IMG_TILED] = 0,

    [LV_STYLE_BORDER_COLOR] = 0,
    [LV_STYLE_BORDER_OPA] = 0,
    [LV_STYLE_BORDER_WIDTH] =              LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_BORDER_SIDE] = 0,
    [LV_STYLE_BORDER_POST] = 0,

    [LV_STYLE_OUTLINE_WIDTH] =             LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_OUTLINE_COLOR] = 0,
    [LV_STYLE_OUTLINE_OPA] =               LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_OUTLINE_PAD] =               LV_STYLE_PROP_EXT_DRAW,

    [LV_STYLE_SHADOW_WIDTH] =              LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_SHADOW_OFS_X] =              LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_SHADOW_OFS_Y] =              LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_SHADOW_SPREAD] =             LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_SHADOW_COLOR] = 0,
    [LV_STYLE_SHADOW_OPA] =                LV_STYLE_PROP_EXT_DRAW,

    [LV_STYLE_IMG_OPA] = 0,
    [LV_STYLE_IMG_RECOLOR] = 0,
    [LV_STYLE_IMG_RECOLOR_OPA] = 0,

    [LV_STYLE_LINE_WIDTH] =                LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_LINE_DASH_WIDTH] = 0,
    [LV_STYLE_LINE_DASH_GAP] = 0,
    [LV_STYLE_LINE_ROUNDED] = 0,
    [LV_STYLE_LINE_COLOR] = 0,
    [LV_STYLE_LINE_OPA] = 0,

    [LV_STYLE_ARC_WIDTH] =                 LV_STYLE_PROP_EXT_DRAW,
    [LV_STYLE_ARC_ROUNDED] = 0,
    [LV_STYLE_ARC_COLOR] = 0,
    [LV_STYLE_ARC_OPA] = 0,
    [LV_STYLE_ARC_IMG_SRC] = 0,

    [LV_STYLE_TEXT_COLOR] =                LV_STYLE_PROP_INHERIT,
    [LV_STYLE_TEXT_OPA] =                  LV_STYLE_PROP_INHERIT,
    [LV_STYLE_TEXT_FONT] =                 LV_STYLE_PROP_INHERIT | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_TEXT_LETTER_SPACE] =         LV_STYLE_PROP_INHERIT | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_TEXT_LINE_SPACE] =           LV_STYLE_PROP_INHERIT | LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_TEXT_DECOR] =                LV_STYLE_PROP_INHERIT,
    [LV_STYLE_TEXT_ALIGN] =                LV_STYLE_PROP_INHERIT | LV_STYLE_PROP_LAYOUT_REFR,

    [LV_STYLE_RADIUS] = 0,
    [LV_STYLE_CLIP_CORNER] = 0,
    [LV_STYLE_OPA] =                       LV_STYLE_PROP_INHERIT,
    [LV_STYLE_COLOR_FILTER_DSC] = 0,
    [LV_STYLE_COLOR_FILTER_OPA] = 0,
    [LV_STYLE_ANIM_TIME] = 0,
    [LV_STYLE_ANIM_SPEED] = 0,
    [LV_STYLE_TRANSITION] = 0,
    [LV_STYLE_BLEND_MODE] = 0,
    [LV_STYLE_LAYOUT] =                    LV_STYLE_PROP_LAYOUT_REFR,
    [LV_STYLE_BASE_DIR] =                  LV_STYLE_PROP_INHERIT | LV_STYLE_PROP_LAYOUT_REFR,
};

uint8_t * lv_style_custom_prop_flag_lookup_table = NULL;

/**********************
 *  STATIC VARIABLES
 **********************/

static uint16_t act_custom_prop_id = (uint16_t)_LV_STYLE_LAST_BUILT_IN_PROP;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_style_init(lv_style_t * style)
{
#if LV_USE_ASSERT_STYLE
    if(style->sentinel == LV_STYLE_SENTINEL_VALUE && style->prop_cnt > 1) {
        LV_LOG_WARN("Style might be already inited. (Potential memory leak)");
    }
#endif

    lv_memset_00(style, sizeof(lv_style_t));
#if LV_USE_ASSERT_STYLE
    style->sentinel = LV_STYLE_SENTINEL_VALUE;
#endif
}

void lv_style_reset(lv_style_t * style)
{
    LV_ASSERT_STYLE(style);

    if(style->is_const) {
        LV_LOG_ERROR("Cannot reset const style");
        return;
    }

    if(style->prop_cnt > 1) lv_mem_free(style->v_p.values_and_props);
    lv_memset_00(style, sizeof(lv_style_t));
#if LV_USE_ASSERT_STYLE
    style->sentinel = LV_STYLE_SENTINEL_VALUE;
#endif
}

lv_style_prop_t lv_style_register_prop(uint8_t flag)
{
    /*
     * Allocate the lookup table if it's not yet available.
     */
    if(lv_style_custom_prop_flag_lookup_table == NULL) {
        lv_style_custom_prop_flag_lookup_table = lv_mem_alloc(LV_STYLE_MAX_CUSTOM_PROPS);
        if(lv_style_custom_prop_flag_lookup_table == NULL) {
            LV_LOG_ERROR("Unable to allocate space for custom property lookup table");
            return LV_STYLE_PROP_INV;
        }
    }
    act_custom_prop_id++;
    /*
     * This ID is the true property ID passed to users, so compare it to the expected
     * total number of properties, not just custom ones.
     */
    if(act_custom_prop_id >= _LV_STYLE_NUM_TOTAL_PROPS) {
        /* Revert the addition */
        act_custom_prop_id--;
        LV_LOG_ERROR("Too many custom properties registered");
        return LV_STYLE_PROP_INV;
    }
    lv_style_custom_prop_flag_lookup_table[act_custom_prop_id - _LV_STYLE_NUM_BUILT_IN_PROPS] = flag;
    return act_custom_prop_id;
}

lv_style_prop_t lv_style_get_num_custom_props(void)
{
    return act_custom_prop_id - _LV_STYLE_LAST_BUILT_IN_PROP;
}

bool lv_style_remove_prop(lv_style_t * style, lv_style_prop_t prop)
{
    LV_ASSERT_STYLE(style);

    if(style->is_const) {
        LV_LOG_ERROR("Cannot remove prop from const style");
        return false;
    }

    if(style->prop_cnt == 0)  return false;

    if(style->prop_cnt == 1) {
        if(style->prop1 == prop) {
            style->prop1 = LV_STYLE_PROP_INV;
            style->prop_cnt = 0;
            return true;
        }
        return false;
    }

    uint8_t * tmp = style->v_p.values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
    uint16_t * old_props = (uint16_t *)tmp;
    uint32_t i;
    for(i = 0; i < style->prop_cnt; i++) {
        if(old_props[i] == prop) {
            lv_style_value_t * old_values = (lv_style_value_t *)style->v_p.values_and_props;

            if(style->prop_cnt == 2) {
                style->prop_cnt = 1;
                style->prop1 = i == 0 ? old_props[1] : old_props[0];
                style->v_p.value1 = i == 0 ? old_values[1] : old_values[0];
            }
            else {
                size_t size = (style->prop_cnt - 1) * (sizeof(lv_style_value_t) + sizeof(uint16_t));
                uint8_t * new_values_and_props = lv_mem_alloc(size);
                if(new_values_and_props == NULL) return false;
                style->v_p.values_and_props = new_values_and_props;
                style->prop_cnt--;

                tmp = new_values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
                uint16_t * new_props = (uint16_t *)tmp;
                lv_style_value_t * new_values = (lv_style_value_t *)new_values_and_props;

                uint32_t j;
                for(i = j = 0; j <= style->prop_cnt;
                    j++) { /*<=: because prop_cnt already reduced but all the old props. needs to be checked.*/
                    if(old_props[j] != prop) {
                        new_values[i] = old_values[j];
                        new_props[i++] = old_props[j];
                    }
                }
            }

            lv_mem_free(old_values);
            return true;
        }
    }

    return false;
}

void lv_style_set_prop(lv_style_t * style, lv_style_prop_t prop, lv_style_value_t value)
{
    LV_ASSERT_STYLE(style);

    if(style->is_const) {
        LV_LOG_ERROR("Cannot set property of constant style");
        return;
    }

    if(style->prop_cnt > 1) {
        uint8_t * tmp = style->v_p.values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
        uint16_t * props = (uint16_t *)tmp;
        int32_t i;
        for(i = style->prop_cnt - 1; i >= 0; i--) {
            if(props[i] == prop) {
                lv_style_value_t * values = (lv_style_value_t *)style->v_p.values_and_props;
                values[i] = value;
                return;
            }
        }

        size_t size = (style->prop_cnt + 1) * (sizeof(lv_style_value_t) + sizeof(uint16_t));
        uint8_t * values_and_props = lv_mem_realloc(style->v_p.values_and_props, size);
        if(values_and_props == NULL) return;
        style->v_p.values_and_props = values_and_props;

        tmp = values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
        props = (uint16_t *)tmp;
        /*Shift all props to make place for the value before them*/
        for(i = style->prop_cnt - 1; i >= 0; i--) {
            props[i + sizeof(lv_style_value_t) / sizeof(uint16_t)] = props[i];
        }
        style->prop_cnt++;

        /*Go to the new position wit the props*/
        tmp = values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
        props = (uint16_t *)tmp;
        lv_style_value_t * values = (lv_style_value_t *)values_and_props;

        /*Set the new property and value*/
        props[style->prop_cnt - 1] = prop;
        values[style->prop_cnt - 1] = value;
    }
    else if(style->prop_cnt == 1) {
        if(style->prop1 == prop) {
            style->v_p.value1 = value;
            return;
        }
        size_t size = (style->prop_cnt + 1) * (sizeof(lv_style_value_t) + sizeof(uint16_t));
        uint8_t * values_and_props = lv_mem_alloc(size);
        if(values_and_props == NULL) return;
        lv_style_value_t value_tmp = style->v_p.value1;
        style->v_p.values_and_props = values_and_props;
        style->prop_cnt++;

        uint8_t * tmp = values_and_props + style->prop_cnt * sizeof(lv_style_value_t);
        uint16_t * props = (uint16_t *)tmp;
        lv_style_value_t * values = (lv_style_value_t *)values_and_props;
        props[0] = style->prop1;
        props[1] = prop;
        values[0] = value_tmp;
        values[1] = value;
    }
    else {
        style->prop_cnt = 1;
        style->prop1 = prop;
        style->v_p.value1 = value;
    }

    uint8_t group = _lv_style_get_prop_group(prop);
    style->has_group |= 1 << group;
}

lv_res_t lv_style_get_prop(const lv_style_t * style, lv_style_prop_t prop, lv_style_value_t * value)
{
    return lv_style_get_prop_inlined(style, prop, value);
}

void lv_style_transition_dsc_init(lv_style_transition_dsc_t * tr, const lv_style_prop_t props[],
                                  lv_anim_path_cb_t path_cb, uint32_t time, uint32_t delay, void * user_data)
{
    lv_memset_00(tr, sizeof(lv_style_transition_dsc_t));
    tr->props = props;
    tr->path_xcb = path_cb == NULL ? lv_anim_path_linear : path_cb;
    tr->time = time;
    tr->delay = delay;
#if LV_USE_USER_DATA
    tr->user_data = user_data;
#else
    LV_UNUSED(user_data);
#endif
}

lv_style_value_t lv_style_prop_get_default(lv_style_prop_t prop)
{
    lv_style_value_t value;
    switch(prop) {
        case LV_STYLE_TRANSFORM_ZOOM:
            value.num = LV_IMG_ZOOM_NONE;
            break;
        case LV_STYLE_BG_COLOR:
            value.color = lv_color_white();
            break;
        case LV_STYLE_BG_GRAD_COLOR:
        case LV_STYLE_BORDER_COLOR:
        case LV_STYLE_SHADOW_COLOR:
        case LV_STYLE_OUTLINE_COLOR:
        case LV_STYLE_ARC_COLOR:
        case LV_STYLE_LINE_COLOR:
        case LV_STYLE_TEXT_COLOR:
        case LV_STYLE_IMG_RECOLOR:
            value.color = lv_color_black();
            break;
        case LV_STYLE_OPA:
        case LV_STYLE_BORDER_OPA:
        case LV_STYLE_TEXT_OPA:
        case LV_STYLE_IMG_OPA:
        case LV_STYLE_BG_IMG_OPA:
        case LV_STYLE_OUTLINE_OPA:
        case LV_STYLE_SHADOW_OPA:
        case LV_STYLE_LINE_OPA:
        case LV_STYLE_ARC_OPA:
            value.num = LV_OPA_COVER;
            break;
        case LV_STYLE_BG_GRAD_STOP:
            value.num = 255;
            break;
        case LV_STYLE_BORDER_SIDE:
            value.num = LV_BORDER_SIDE_FULL;
            break;
        case LV_STYLE_TEXT_FONT:
            value.ptr = LV_FONT_DEFAULT;
            break;
        case LV_STYLE_MAX_WIDTH:
        case LV_STYLE_MAX_HEIGHT:
            value.num = LV_COORD_MAX;
            break;
        default:
            value.ptr = NULL;
            value.num = 0;
            break;
    }

    return value;
}

bool lv_style_is_empty(const lv_style_t * style)
{
    LV_ASSERT_STYLE(style);

    return style->prop_cnt == 0 ? true : false;
}

uint8_t _lv_style_get_prop_group(lv_style_prop_t prop)
{
    uint16_t group = (prop & 0x1FF) >> 4;
    if(group > 7) group = 7;    /*The MSB marks all the custom properties*/
    return (uint8_t)group;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
