/**
 * @file lv_draw_sw_blend_to_i1.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_sw_blend_to_i1.h"
#if LV_USE_DRAW_SW

#include "lv_draw_sw_blend.h"
#include "../../../misc/lv_math.h"
#include "../../../display/lv_display.h"
#include "../../../core/lv_refr.h"
#include "../../../misc/lv_color.h"
#include "../../../stdlib/lv_string.h"

#if LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_NEON
    #include "neon/lv_blend_neon.h"
#elif LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_HELIUM
    #include "helium/lv_blend_helium.h"
#elif LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_CUSTOM
    #include LV_DRAW_SW_ASM_CUSTOM_INCLUDE
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void /* LV_ATTRIBUTE_FAST_MEM */ i1_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc);

static void /* LV_ATTRIBUTE_FAST_MEM */ l8_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc);

static void /* LV_ATTRIBUTE_FAST_MEM */ al88_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc);

static void /* LV_ATTRIBUTE_FAST_MEM */ rgb565_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc);

static void /* LV_ATTRIBUTE_FAST_MEM */ rgb888_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc,
                                                           const uint8_t src_px_size);

static void /* LV_ATTRIBUTE_FAST_MEM */ argb8888_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc);

static inline void /* LV_ATTRIBUTE_FAST_MEM */ lv_color_8_8_mix(const uint8_t src, uint8_t * dest, uint8_t mix);

static inline void /* LV_ATTRIBUTE_FAST_MEM */ lv_color_1bit_blend(const uint8_t src, uint8_t * dest, uint8_t mix);

static inline void /* LV_ATTRIBUTE_FAST_MEM */ blend_non_normal_pixel(uint8_t * dest, lv_color32_t src,
                                                                      lv_blend_mode_t mode);

static inline void * /* LV_ATTRIBUTE_FAST_MEM */ drawbuf_next_row(const void * buf, uint32_t stride);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifndef LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1
    #define LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1(...)                    LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_OPA
    #define LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_OPA(...)           LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_MASK
    #define LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_MASK(...)          LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(...)       LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_COLOR_BLEND_TO_I1
    #define LV_DRAW_SW_COLOR_BLEND_TO_I1(...)                         LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_OPA
    #define LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_OPA(...)                LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_MASK
    #define LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_MASK(...)               LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_COLOR_BLEND_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_COLOR_BLEND_TO_I1_MIX_MASK_OPA(...)            LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1
    #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1(...)                 LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_WITH_OPA
    #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_WITH_OPA(...)        LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_WITH_MASK
    #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_WITH_MASK(...)       LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(...)    LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1
    #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1(...)                 LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_WITH_OPA
    #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_WITH_OPA(...)        LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_WITH_MASK
    #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_WITH_MASK(...)       LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(...)    LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1
    #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1(...)               LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_OPA
    #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_OPA(...)      LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_MASK
    #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_MASK(...)     LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(...)  LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1
    #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1(...)               LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_WITH_OPA
    #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_WITH_OPA(...)      LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_WITH_MASK
    #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_WITH_MASK(...)     LV_RESULT_INVALID
#endif

#ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_MIX_MASK_OPA
    #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(...)  LV_RESULT_INVALID
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void LV_ATTRIBUTE_FAST_MEM lv_draw_sw_blend_color_to_i1(_lv_draw_sw_blend_fill_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    const lv_opa_t * mask = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;
    int32_t dest_stride = dsc->dest_stride;

    int32_t x;
    int32_t y;

    LV_UNUSED(w);
    LV_UNUSED(h);
    LV_UNUSED(x);
    LV_UNUSED(y);
    LV_UNUSED(opa);
    LV_UNUSED(mask);
    LV_UNUSED(mask_stride);
    LV_UNUSED(dest_stride);

    uint8_t color1 = (lv_color_luminance(dsc->color) >= 128) ? 1 : 0;
    uint8_t * dest_buf = dsc->dest_buf;

    /* Simple fill */
    if(mask == NULL && opa >= LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_DRAW_SW_COLOR_BLEND_TO_I1(dsc)) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    if(color1) {
                        dest_buf[x / 8] |= (1 << (7 - (x % 8))); // Set bit to 1
                    }
                    else {
                        dest_buf[x / 8] &= ~(1 << (7 - (x % 8))); // Set bit to 0
                    }
                }
                dest_buf = drawbuf_next_row(dest_buf, dest_stride);
            }
        }
    }
    /* Opacity only */
    else if(mask == NULL && opa < LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_OPA(dsc)) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    uint8_t * dest_bit = &dest_buf[x / 8];
                    lv_color_1bit_blend(color1, dest_bit, opa);
                }
                dest_buf = drawbuf_next_row(dest_buf, dest_stride);
            }
        }
    }
    /* Masked with full opacity */
    else if(mask && opa >= LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_DRAW_SW_COLOR_BLEND_TO_I1_WITH_MASK(dsc)) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    uint8_t mask_val = mask[x];
                    if(mask_val == LV_OPA_TRANSP) continue;
                    uint8_t * dest_bit = &dest_buf[x / 8];
                    if(mask_val == LV_OPA_COVER) {
                        if(color1) {
                            *dest_bit |= (1 << (7 - (x % 8))); // Set bit to 1
                        }
                        else {
                            *dest_bit &= ~(1 << (7 - (x % 8))); // Set bit to 0
                        }
                    }
                    else {
                        lv_color_1bit_blend(color1, dest_bit, mask_val);
                    }
                }
                dest_buf = drawbuf_next_row(dest_buf, dest_stride);
                mask += mask_stride;
            }
        }
    }
    /* Masked with opacity */
    else {
        if(LV_RESULT_INVALID == LV_DRAW_SW_COLOR_BLEND_TO_I1_MIX_MASK_OPA(dsc)) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    uint8_t mask_val = mask[x];
                    if(mask_val == LV_OPA_TRANSP) continue;
                    uint8_t * dest_bit = &dest_buf[x / 8];
                    lv_color_1bit_blend(color1, dest_bit, LV_OPA_MIX2(mask_val, opa));
                }
                dest_buf = drawbuf_next_row(dest_buf, dest_stride);
                mask += mask_stride;
            }
        }
    }
}

void LV_ATTRIBUTE_FAST_MEM lv_draw_sw_blend_image_to_i1(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    switch(dsc->src_color_format) {
        case LV_COLOR_FORMAT_RGB565:
            rgb565_image_blend(dsc);
            break;
        case LV_COLOR_FORMAT_RGB888:
            rgb888_image_blend(dsc, 3);
            break;
        case LV_COLOR_FORMAT_XRGB8888:
            rgb888_image_blend(dsc, 4);
            break;
        case LV_COLOR_FORMAT_ARGB8888:
            argb8888_image_blend(dsc);
            break;
        case LV_COLOR_FORMAT_L8:
            l8_image_blend(dsc);
            break;
        case LV_COLOR_FORMAT_AL88:
            al88_image_blend(dsc);
            break;
        case LV_COLOR_FORMAT_I1:
            i1_image_blend(dsc);
            break;
        default:
            LV_LOG_WARN("Not supported source color format");
            break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void LV_ATTRIBUTE_FAST_MEM i1_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_i1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_i1 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1(dsc)) {
                for(y = 0; y < h; y++) {
                    for(src_x = 0; src_x < w; src_x++) {
                        dest_x = src_x;
                        if(src_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) {
                            dest_buf_i1[dest_x / 8] |= (1 << (7 - (dest_x % 8))); // Set bit to 1
                        }
                        else {
                            dest_buf_i1[dest_x / 8] &= ~(1 << (7 - (dest_x % 8))); // Set bit to 0
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                    src_buf_i1 = drawbuf_next_row(src_buf_i1, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_OPA(dsc)) {
                for(y = 0; y < h; y++) {
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = (src_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 1 : 0;
                        uint8_t * dest = &dest_buf_i1[dest_x / 8];
                        lv_color_1bit_blend(src, dest, opa);
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                    src_buf_i1 = drawbuf_next_row(src_buf_i1, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_WITH_MASK(dsc)) {
                for(y = 0; y < h; y++) {
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = (src_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 1 : 0;
                        uint8_t mask_val = mask_buf[src_x];
                        if(mask_val == LV_OPA_TRANSP) continue;
                        uint8_t * dest = &dest_buf_i1[dest_x / 8];
                        if(mask_val == LV_OPA_COVER) {
                            if(src) {
                                *dest |= (1 << (7 - (dest_x % 8))); // Set bit to 1
                            }
                            else {
                                *dest &= ~(1 << (7 - (dest_x % 8))); // Set bit to 0
                            }
                        }
                        else {
                            lv_color_1bit_blend(src, dest, mask_val);
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                    src_buf_i1 = drawbuf_next_row(src_buf_i1, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_I1_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(dsc)) {
                for(y = 0; y < h; y++) {
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = (src_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 1 : 0;
                        uint8_t mask_val = mask_buf[src_x];
                        if(mask_val == LV_OPA_TRANSP) continue;
                        uint8_t * dest = &dest_buf_i1[dest_x / 8];
                        lv_color_1bit_blend(src, dest, LV_OPA_MIX2(mask_val, opa));
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                    src_buf_i1 = drawbuf_next_row(src_buf_i1, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                src_argb.red = src_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8))) ? 255 : 0;
                src_argb.green = src_argb.red;
                src_argb.blue = src_argb.red;
                if(mask_buf == NULL) {
                    src_argb.alpha = opa;
                }
                else {
                    src_argb.alpha = LV_OPA_MIX2(mask_buf[dest_x], opa);
                }
                blend_non_normal_pixel(&dest_buf_i1[dest_x / 8], src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
            src_buf_i1 = drawbuf_next_row(src_buf_i1, src_stride);
        }
    }
}

static void LV_ATTRIBUTE_FAST_MEM l8_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_i1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_l8 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t src_x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1(dsc)) {
                for(y = 0; y < h; y++) {
                    for(src_x = 0; src_x < w; src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t bit_pos = 7 - (src_x % 8);
                        if(src_luminance > 127) {
                            dest_buf_i1[src_x / 8] |= (1 << bit_pos); // Set bit
                        }
                        else {
                            dest_buf_i1[src_x / 8] &= ~(1 << bit_pos); // Clear bit
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride / 8);
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_OPA(dsc)) {
                for(y = 0; y < h; y++) {
                    for(src_x = 0; src_x < w; src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t dest_val = (dest_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 255 : 0;
                        lv_color_8_8_mix(src_luminance, &dest_val, opa);
                        uint8_t bit_pos = 7 - (src_x % 8);
                        if(dest_val > 127) {
                            dest_buf_i1[src_x / 8] |= (1 << bit_pos); // Set bit
                        }
                        else {
                            dest_buf_i1[src_x / 8] &= ~(1 << bit_pos); // Clear bit
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride / 8);
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_WITH_MASK(dsc)) {
                for(y = 0; y < h; y++) {
                    for(src_x = 0; src_x < w; src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t mask_val = mask_buf[src_x];
                        uint8_t dest_val = (dest_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 255 : 0;
                        lv_color_8_8_mix(src_luminance, &dest_val, mask_val);
                        uint8_t bit_pos = 7 - (src_x % 8);
                        if(dest_val > 127) {
                            dest_buf_i1[src_x / 8] |= (1 << bit_pos); // Set bit
                        }
                        else {
                            dest_buf_i1[src_x / 8] &= ~(1 << bit_pos); // Clear bit
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride / 8);
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_DRAW_SW_L8_BLEND_NORMAL_TO_I1_MIX_MASK_OPA(dsc)) {
                for(y = 0; y < h; y++) {
                    for(src_x = 0; src_x < w; src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t mask_val = mask_buf[src_x];
                        uint8_t dest_val = (dest_buf_i1[src_x / 8] & (1 << (7 - (src_x % 8)))) ? 255 : 0;
                        lv_color_8_8_mix(src_luminance, &dest_val, LV_OPA_MIX2(mask_val, opa));
                        uint8_t bit_pos = 7 - (src_x % 8);
                        if(dest_val > 127) {
                            dest_buf_i1[src_x / 8] |= (1 << bit_pos); // Set bit
                        }
                        else {
                            dest_buf_i1[src_x / 8] &= ~(1 << bit_pos); // Clear bit
                        }
                    }
                    dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride / 8);
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            for(src_x = 0; src_x < w; src_x++) {
                src_argb.red = src_buf_l8[src_x];
                src_argb.green = src_buf_l8[src_x];
                src_argb.blue = src_buf_l8[src_x];
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[src_x], opa);
                blend_non_normal_pixel(&dest_buf_i1[src_x / 8], src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride / 8);
            src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
        }
    }
}

static void LV_ATTRIBUTE_FAST_MEM al88_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_i1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color16a_t * src_buf_al88 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                    uint8_t pixel = src_buf_al88[src_x].lumi >= 128 ? 0xFF : 0x00;
                    lv_color_1bit_blend(pixel, &dest_buf_i1[dest_x], opa);
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                    uint8_t pixel = src_buf_al88[src_x].lumi >= 128 ? 0xFF : 0x00;
                    lv_color_1bit_blend(pixel, &dest_buf_i1[dest_x], LV_OPA_MIX2(src_buf_al88[src_x].alpha, opa));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                    uint8_t pixel = src_buf_al88[src_x].lumi >= 128 ? 0xFF : 0x00;
                    lv_color_1bit_blend(pixel, &dest_buf_i1[dest_x], LV_OPA_MIX2(src_buf_al88[src_x].alpha, mask_buf[src_x]));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                mask_buf += mask_stride;
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                    uint8_t pixel = src_buf_al88[src_x].lumi >= 128 ? 0xFF : 0x00;
                    lv_color_1bit_blend(pixel, &dest_buf_i1[dest_x], LV_OPA_MIX3(src_buf_al88[src_x].alpha, mask_buf[src_x], opa));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                mask_buf += mask_stride;
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                // Convert AL88 pixel to ARGB
                src_argb.red = src_buf_al88[src_x].lumi;
                src_argb.green = src_buf_al88[src_x].lumi;
                src_argb.blue = src_buf_al88[src_x].lumi;
                // If no mask, use specified opacity, otherwise mix with mask opacity
                src_argb.alpha = mask_buf == NULL ? opa : LV_OPA_MIX2(mask_buf[src_x], opa);
                // Blend pixel using specified blend mode
                blend_non_normal_pixel(&dest_buf_i1[dest_x], src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
            src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
        }
    }
}

static void LV_ATTRIBUTE_FAST_MEM argb8888_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_i1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color32_t * src_buf_c32 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    lv_color_1bit_blend(lv_color32_luminance(src_buf_c32[x]), &dest_buf_i1[x], src_buf_c32[x].alpha);
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    lv_color_1bit_blend(lv_color32_luminance(src_buf_c32[x]), &dest_buf_i1[x], LV_OPA_MIX2(src_buf_c32[x].alpha, opa));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    lv_color_1bit_blend(lv_color32_luminance(src_buf_c32[x]), &dest_buf_i1[x], LV_OPA_MIX2(src_buf_c32[x].alpha,
                                                                                                           mask_buf[x]));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                mask_buf += mask_stride;
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++) {
                    lv_color_1bit_blend(lv_color32_luminance(src_buf_c32[x]), &dest_buf_i1[x], LV_OPA_MIX3(src_buf_c32[x].alpha, opa,
                                                                                                           mask_buf[x]));
                }
                dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
                src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                mask_buf += mask_stride;
            }
        }
    }
    else {
        for(y = 0; y < h; y++) {
            for(x = 0; x < w; x++) {
                lv_color32_t color_argb = src_buf_c32[x];
                if(mask_buf == NULL) {
                    color_argb.alpha = LV_OPA_MIX2(color_argb.alpha, opa);
                }
                else {
                    color_argb.alpha = LV_OPA_MIX3(color_argb.alpha, mask_buf[x], opa);
                }
                blend_non_normal_pixel(&dest_buf_i1[x], color_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_i1 = drawbuf_next_row(dest_buf_i1, dest_stride);
            src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
        }
    }
}

static void LV_ATTRIBUTE_FAST_MEM rgb888_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc,
                                                     const uint8_t src_px_size)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_i1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_u8 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        /*Special case*/
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                    dest_buf_i1[dest_x] = lv_color24_luminance(&src_buf_u8[src_x]);
                }
                dest_buf_i1 += dest_stride;
                src_buf_u8 += src_stride;
            }
        }
        if(mask_buf == NULL && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                    lv_color_1bit_blend(lv_color24_luminance(&src_buf_u8[src_x]), &dest_buf_i1[dest_x], opa);
                }
                dest_buf_i1 += dest_stride;
                src_buf_u8 += src_stride;
            }
        }
        if(mask_buf && opa >= LV_OPA_MAX) {
            uint32_t mask_x;
            for(y = 0; y < h; y++) {
                for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x += src_px_size) {
                    lv_color_1bit_blend(lv_color24_luminance(&src_buf_u8[src_x]), &dest_buf_i1[dest_x], mask_buf[mask_x]);
                }
                dest_buf_i1 += dest_stride;
                src_buf_u8 += src_stride;
                mask_buf += mask_stride;
            }
        }
        if(mask_buf && opa < LV_OPA_MAX) {
            uint32_t mask_x;
            for(y = 0; y < h; y++) {
                for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x += src_px_size) {
                    lv_color_1bit_blend(lv_color24_luminance(&src_buf_u8[src_x]), &dest_buf_i1[dest_x], LV_OPA_MIX2(opa, mask_buf[mask_x]));
                }
                dest_buf_i1 += dest_stride;
                src_buf_u8 += src_stride;
                mask_buf += mask_stride;
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                src_argb.red = src_buf_u8[src_x + 2];
                src_argb.green = src_buf_u8[src_x + 1];
                src_argb.blue = src_buf_u8[src_x + 0];
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[dest_x], opa);

                blend_non_normal_pixel(&dest_buf_i1[dest_x], src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_i1 += dest_stride;
            src_buf_u8 += src_stride;
        }
    }
}

static void LV_ATTRIBUTE_FAST_MEM rgb565_image_blend(_lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_u8 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color16_t * src_buf_c16 = (const lv_color16_t *)dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t src_x;
    int32_t dest_x;
    int32_t y;

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                    dest_buf_u8[dest_x] = lv_color16_luminance(src_buf_c16[src_x]);
                }
                dest_buf_u8 += dest_stride;
                src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            for(y = 0; y < h; y++) {
                for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                    lv_color_1bit_blend(lv_color16_luminance(src_buf_c16[src_x]), &dest_buf_u8[dest_x], opa);
                }
                dest_buf_u8 += dest_stride;
                src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            uint32_t mask_x;
            for(y = 0; y < h; y++) {
                for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                    lv_color_1bit_blend(lv_color16_luminance(src_buf_c16[src_x]), &dest_buf_u8[dest_x], mask_buf[mask_x]);
                }
                dest_buf_u8 += dest_stride;
                src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                mask_buf += mask_stride;
            }
        }
        else {
            uint32_t mask_x;
            for(y = 0; y < h; y++) {
                for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                    lv_color_1bit_blend(lv_color16_luminance(src_buf_c16[src_x]), &dest_buf_u8[dest_x], LV_OPA_MIX2(opa, mask_buf[mask_x]));
                }
                dest_buf_u8 += dest_stride;
                src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                mask_buf += mask_stride;
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            for(src_x = 0, dest_x = 0; src_x < w; src_x++, dest_x++) {
                src_argb.red = (src_buf_c16[src_x].red * 2106) >> 8;
                src_argb.green = (src_buf_c16[src_x].green * 1037) >> 8;
                src_argb.blue = (src_buf_c16[src_x].blue * 2106) >> 8;
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[src_x], opa);

                blend_non_normal_pixel(&dest_buf_u8[dest_x], src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            dest_buf_u8 += dest_stride;
            src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
        }
    }
}


static inline void LV_ATTRIBUTE_FAST_MEM blend_non_normal_pixel(uint8_t * dest, lv_color32_t src, lv_blend_mode_t mode)
{
    uint8_t res;
    int32_t src_lumi = lv_color32_luminance(src);
    uint8_t dest_val = (*dest & (1 << (7 - (*dest & 7)))) ? 255 : 0; // Convert the I1 bit to full 0 or 255 luminance
    switch(mode) {
        case LV_BLEND_MODE_ADDITIVE:
            res = LV_MIN(dest_val + src_lumi, 255);
            break;
        case LV_BLEND_MODE_SUBTRACTIVE:
            res = LV_MAX(dest_val - src_lumi, 0);
            break;
        case LV_BLEND_MODE_MULTIPLY:
            res = (dest_val * src_lumi) >> 8;
            break;
        default:
            LV_LOG_WARN("Not supported blend mode: %d", mode);
            return;
    }
    lv_color_1bit_blend(res, &dest_val, src.alpha);
    if(dest_val) {
        *dest |= (1 << (7 - (*dest & 7))); // Set bit to 1
    }
    else {
        *dest &= ~(1 << (7 - (*dest & 7))); // Set bit to 0
    }
}

static inline void LV_ATTRIBUTE_FAST_MEM lv_color_8_8_mix(const uint8_t src, uint8_t * dest, uint8_t mix)
{

    if(mix == 0) return;

    if(mix >= LV_OPA_MAX) {
        *dest = src;
    }
    else {
        lv_opa_t mix_inv = 255 - mix;
        *dest = (uint32_t)((uint32_t)src * mix + dest[0] * mix_inv) >> 8;
    }
}

static inline void LV_ATTRIBUTE_FAST_MEM lv_color_1bit_blend(const uint8_t src, uint8_t * dest, uint8_t mix)
{
    uint8_t bg_color = (*dest) ? 1 : 0;
    *dest = (mix >= LV_OPA_MAX) ? src : ((mix == LV_OPA_TRANSP) ? bg_color : ((src * mix + bg_color * (255 - mix)) >> 8));
}

static inline void * LV_ATTRIBUTE_FAST_MEM drawbuf_next_row(const void * buf, uint32_t stride)
{
    uint32_t bytesPerRow = (stride + 7) / 8;
    return (void *)((uint8_t *)buf + bytesPerRow);
}

#endif
