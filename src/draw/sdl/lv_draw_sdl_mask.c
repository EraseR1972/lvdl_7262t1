/**
 * @file lv_draw_sdl_mask.c
 *
 */

/*********************
 *      INCLUDES
 *********************/


#include <src/misc/lv_gc.h>
#include "../../lv_conf_internal.h"

#if LV_USE_DRAW_SDL

#include "../../core/lv_refr.h"
#include "../lv_draw_mask.h"
#include "lv_draw_sdl_mask.h"
#include "lv_draw_sdl_utils.h"
#include "lv_draw_sdl_priv.h"

/*********************
 *      DEFINES
 *********************/
#define HAS_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
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

bool lv_draw_sdl_mask_begin(const lv_area_t *coords_in, const lv_area_t *clip_in, const lv_area_t *extension,
                            lv_area_t *coords_out, lv_area_t *clip_out, lv_area_t *apply_area) {
    lv_area_t full_area, full_coords = *coords_in;
    if (extension) {
        full_coords.x1 -= extension->x1;
        full_coords.x2 += extension->x2;
        full_coords.y1 -= extension->y1;
        full_coords.y2 += extension->y2;
    }
    if (!_lv_area_intersect(&full_area, &full_coords, clip_in)) return false;
    if (!lv_draw_mask_is_any(&full_area)) return false;
#if HAS_CUSTOM_BLEND_MODE
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();

    lv_draw_sdl_context_t *context = disp->driver->user_data;
    lv_draw_sdl_context_internals_t *internals = context->internals;
    LV_ASSERT(internals->mask == NULL && internals->composition == NULL);
    internals->mask = lv_sdl_gen_mask_texture(context->renderer, &full_area, NULL, 0);
    *apply_area = full_area;
    lv_coord_t w = lv_area_get_width(&full_area), h = lv_area_get_height(&full_area);
    /* Don't need to worry about overflow */
    lv_coord_t ofs_x = (lv_coord_t) -full_area.x1, ofs_y = (lv_coord_t) -full_area.y1;
    /* Offset draw area to start with (0,0) of coords */
    lv_area_move(&full_area, ofs_x, ofs_y);
    lv_area_move(coords_out, ofs_x, ofs_y);
    lv_area_move(clip_out, ofs_x, ofs_y);
    internals->composition = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_ARGB8888,
                                               SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetRenderTarget(context->renderer, internals->composition);
    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 0);
    SDL_RenderClear(context->renderer);
#else
    /* Fallback mask handling. This will at least make bars looks less bad */
    for (uint8_t i = 0; i < _LV_MASK_MAX_NUM; i++) {
        _lv_draw_mask_common_dsc_t *comm_param = LV_GC_ROOT(_lv_draw_mask_list[i]).param;
        if (comm_param == NULL) continue;
        switch (comm_param->type) {
            case LV_DRAW_MASK_TYPE_RADIUS: {
                const lv_draw_mask_radius_param_t *param = (const lv_draw_mask_radius_param_t *) comm_param;
                if (param->cfg.outer) break;
                const lv_area_t area = *draw_area;
                _lv_area_intersect(draw_area, &area, &param->cfg.rect);
                break;
            }
            default:
                break;
        }
    }
#endif
    return true;
}

void lv_draw_sdl_mask_end(const lv_area_t *apply_area)
{
#if HAS_CUSTOM_BLEND_MODE
    lv_draw_sdl_context_t *context = lv_draw_sdl_get_context();
    lv_draw_sdl_context_internals_t *internals = context->internals;
    LV_ASSERT(internals->mask != NULL && internals->composition != NULL);
    SDL_BlendMode mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE,
                                                    SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO,
                                                    SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
    SDL_SetTextureBlendMode(internals->mask, mode);
    SDL_RenderCopy(context->renderer, internals->mask, NULL, NULL);

    SDL_Rect dst_rect;
    lv_area_to_sdl_rect(apply_area, &dst_rect);

    SDL_SetRenderTarget(context->renderer, context->texture);
    SDL_SetTextureBlendMode(internals->composition, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(context->renderer, internals->composition, NULL, &dst_rect);

    SDL_DestroyTexture(internals->mask);
    SDL_DestroyTexture(internals->composition);
    internals->mask = internals->composition = NULL;
#endif
}

SDL_Surface * lv_sdl_create_mask_surface(lv_opa_t * pixels, lv_coord_t width, lv_coord_t height, lv_coord_t stride)
{
    SDL_Surface * indexed = SDL_CreateRGBSurfaceFrom(pixels, width, height, 8, stride, 0, 0, 0, 0);
    SDL_SetSurfacePalette(indexed, lv_sdl_get_grayscale_palette(8));
    SDL_Surface * converted = SDL_ConvertSurfaceFormat(indexed, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(indexed);
    return converted;
}

SDL_Texture * lv_sdl_create_mask_texture(SDL_Renderer * renderer, lv_opa_t * pixels, lv_coord_t width,
                                         lv_coord_t height, lv_coord_t stride)
{
    SDL_Surface * indexed = lv_sdl_create_mask_surface(pixels, width, height, stride);
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, indexed);
    SDL_FreeSurface(indexed);
    return texture;
}

lv_opa_t * lv_draw_mask_dump(const lv_area_t * coords, const int16_t * ids, int16_t ids_count)
{
    SDL_assert(coords->x2 >= coords->x1);
    SDL_assert(coords->y2 >= coords->y1);
    lv_coord_t w = lv_area_get_width(coords), h = lv_area_get_height(coords);
    lv_opa_t * mask_buf = lv_mem_buf_get(w * h);
    for(lv_coord_t y = 0; y < h; y++) {
        lv_opa_t * line_buf = &mask_buf[y * w];
        lv_memset_ff(line_buf, w);
        lv_coord_t abs_x = (lv_coord_t) coords->x1, abs_y = (lv_coord_t)(y + coords->y1), len = (lv_coord_t) w;
        lv_draw_mask_res_t res;
        if(ids) {
            res = lv_draw_mask_apply_ids(line_buf, abs_x, abs_y, len, ids, ids_count);
        }
        else {
            res = lv_draw_mask_apply(line_buf, abs_x, abs_y, len);
        }
        if(res == LV_DRAW_MASK_RES_TRANSP) {
            lv_memset_00(line_buf, w);
        }
    }
    return mask_buf;
}

SDL_Surface * lv_sdl_apply_mask_surface(const lv_area_t * coords, const int16_t * ids, int16_t ids_count)
{
    lv_coord_t w = lv_area_get_width(coords), h = lv_area_get_height(coords);

    lv_opa_t * mask_buf = lv_draw_mask_dump(coords, ids, ids_count);
    lv_mem_buf_release(mask_buf);
    return lv_sdl_create_mask_surface(mask_buf, w, h, w);
}

SDL_Texture * lv_sdl_gen_mask_texture(SDL_Renderer * renderer, const lv_area_t * coords, const int16_t * ids,
                                      int16_t ids_count)
{
    SDL_Surface * indexed = lv_sdl_apply_mask_surface(coords, ids, ids_count);
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, indexed);
    SDL_FreeSurface(indexed);
    return texture;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /*LV_USE_DRAW_SDL*/
