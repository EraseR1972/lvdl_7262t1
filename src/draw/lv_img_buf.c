/**
 * @file lv_img_buf.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>
#include <string.h>
#include "lv_img_buf.h"
#include "lv_draw_img.h"
#include "../misc/lv_math.h"
#include "../misc/lv_log.h"
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
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Set the palette color of an indexed image. Valid only for `LV_IMG_CF_INDEXED1/2/4/8`
 * @param dsc pointer to an image descriptor
 * @param id the palette color to set:
 *   - for `LV_IMG_CF_INDEXED1`: 0..1
 *   - for `LV_IMG_CF_INDEXED2`: 0..3
 *   - for `LV_IMG_CF_INDEXED4`: 0..15
 *   - for `LV_IMG_CF_INDEXED8`: 0..255
 * @param c the color to set
 */
void lv_img_buf_set_palette(lv_img_dsc_t * dsc, uint8_t id, lv_color_t c)
{
    LV_UNUSED(dsc);
    LV_UNUSED(id);
    LV_UNUSED(c);
    //    if((dsc->header.cf == LV_IMG_CF_ALPHA_1BIT && id > 1) || (dsc->header.cf == LV_IMG_CF_ALPHA_2BIT && id > 3) ||
    //       (dsc->header.cf == LV_IMG_CF_ALPHA_4BIT && id > 15) || (dsc->header.cf == LV_IMG_CF_ALPHA_8BIT)) {
    //        LV_LOG_WARN("lv_img_buf_set_px_alpha: invalid 'id'");
    //        return;
    //    }
    //
    //    lv_color32_t c32 = lv_color_to32(c);
    //    uint8_t * buf = (uint8_t *)dsc->data;
    //    lv_memcpy(&buf[id * sizeof(c32)], &c32, sizeof(c32));
}

/**
 * Free an allocated image buffer
 * @param dsc image buffer to free
 */
void lv_img_buf_free(lv_img_dsc_t * dsc)
{
    if(dsc != NULL) {
        if(dsc->data != NULL)
            lv_free((void *)dsc->data);

        lv_free(dsc);
    }
}

/**
 * Get the area of a rectangle if its rotated and scaled
 * @param res store the coordinates here
 * @param w width of the rectangle to transform
 * @param h height of the rectangle to transform
 * @param angle angle of rotation
 * @param zoom zoom, (256 no zoom)
 * @param pivot x,y pivot coordinates of rotation
 */
void _lv_img_buf_get_transformed_area(lv_area_t * res, lv_coord_t w, lv_coord_t h, int16_t angle, uint16_t zoom,
                                      const lv_point_t * pivot)
{
#if LV_USE_DRAW_MASKS
    if(angle == 0 && zoom == LV_IMG_ZOOM_NONE) {
        res->x1 = 0;
        res->y1 = 0;
        res->x2 = w - 1;
        res->y2 = h - 1;
        return;
    }

    lv_point_t p[4] = {
        {0, 0},
        {w, 0},
        {0, h},
        {w, h},
    };
    lv_point_transform(&p[0], angle, zoom, pivot);
    lv_point_transform(&p[1], angle, zoom, pivot);
    lv_point_transform(&p[2], angle, zoom, pivot);
    lv_point_transform(&p[3], angle, zoom, pivot);
    res->x1 = LV_MIN4(p[0].x, p[1].x, p[2].x, p[3].x) - 2;
    res->x2 = LV_MAX4(p[0].x, p[1].x, p[2].x, p[3].x) + 2;
    res->y1 = LV_MIN4(p[0].y, p[1].y, p[2].y, p[3].y) - 2;
    res->y2 = LV_MAX4(p[0].y, p[1].y, p[2].y, p[3].y) + 2;

#else
    LV_UNUSED(angle);
    LV_UNUSED(zoom);
    LV_UNUSED(pivot);
    res->x1 = 0;
    res->y1 = 0;
    res->x2 = w - 1;
    res->y2 = h - 1;
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
