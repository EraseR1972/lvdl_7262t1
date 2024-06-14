/**
 * @file wayland.c
 *
 * 2024-6-11 modified by w.hu@iseeway.com, no assurance is provided.
 */

/*********************
 *      INCLUDES
 *********************/
#include "wayland.h"

#if LV_USE_WAYLAND

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

#include <linux/input.h>
#include <linux/input-event-codes.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#if !(LV_WAYLAND_XDG_SHELL)
#error "Please select at least one shell integration for Wayland driver"
#else
#include "protocols/wayland-xdg-shell-client-protocol.h"
#endif

#include "smm.h"
#include "../../core/lv_refr.h"

/*********************
 *      DEFINES
 *********************/
#define LW_COLOR_PX_SIZE ((LV_COLOR_DEPTH + 7) >> 3)
#define LW_DRAW_BUFFER_DIV (8)
#define LW_DMG_CACHE_CAPACITY (48)
#define LW_TAG_LOCAL (0)
#define LW_TAG_BUFFER_DAMAGE (1)

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
#define LW_TITLE_BAR_HEIGHT 32
#define LW_BORDER_SIZE 3
#define LW_BUTTON_MARGIN LV_MAX((LW_TITLE_BAR_HEIGHT / 5), LW_BORDER_SIZE)
#define LW_BUTTON_PADDING LV_MAX((LW_TITLE_BAR_HEIGHT / 8), LW_BORDER_SIZE)
#define LW_BUTTON_SIZE (LW_TITLE_BAR_HEIGHT - (2 * LW_BUTTON_MARGIN))
#endif

#ifndef LW_CYCLE_PERIOD
#define LW_CYCLE_PERIOD LV_MIN(LV_DEF_REFR_PERIOD, 5)
#endif

/**********************
 *      TYPEDEFS
 **********************/

enum lw_object_type {
    LW_OBJECT_TITLEBAR = 0,
    LW_OBJECT_BUTTON_CLOSE,
#if LV_WAYLAND_XDG_SHELL
    LW_OBJECT_BUTTON_MAXIMIZE,
    LW_OBJECT_BUTTON_MINIMIZE,
#endif
    LW_OBJECT_BORDER_TOP,
    LW_OBJECT_BORDER_BOTTOM,
    LW_OBJECT_BORDER_LEFT,
    LW_OBJECT_BORDER_RIGHT,

    LW_OBJECT_WINDOW,
};

#define LW_FIRST_DECORATION (LW_OBJECT_TITLEBAR)
#define LW_LAST_DECORATION (LW_OBJECT_BORDER_RIGHT)
#define LW_NUM_DECORATIONS (LW_LAST_DECORATION - LW_FIRST_DECORATION + 1)

struct lw_input {
    struct {
        int x;
        int y;
        lv_indev_state_t left_button;
        lv_indev_state_t right_button;
        lv_indev_state_t wheel_button;
        int16_t wheel_diff;
    } pointer;

    struct {
        lv_key_t key;
        lv_indev_state_t state;
    } keyboard;

    struct {
        int x;
        int y;
        lv_indev_state_t state;
    } touch;
};

struct lw_seat {
    struct wl_touch* wl_touch;
    struct wl_pointer* wl_pointer;
    struct wl_keyboard* wl_keyboard;

    struct {
        struct xkb_keymap* keymap;
        struct xkb_state* state;
    } xkb;
};

typedef struct _lw_window lw_window_t;
struct lw_surface_object {
    lw_window_t* window;

    struct wl_surface* surface;
    bool surface_configured;
    smm_buffer_t* pending_buffer;
    smm_group_t* buffer_group;
    struct wl_subsurface* subsurface;

    enum lw_object_type type;
    int width;
    int height;

    struct lw_input input;
};

struct lw_application {
    struct wl_display* wl_disp;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_subcompositor* subcompositor;
    struct wl_shm* shm;
    struct wl_seat* wl_seat;

    struct wl_cursor_theme* cursor_theme;
    struct wl_surface* cursor_surface;

#if LV_WAYLAND_XDG_SHELL
    struct xdg_wm_base* xdg_wm;
#endif

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    bool opt_disable_decorations;
#endif

    uint32_t format;

    struct xkb_context* xkb_context;
    struct lw_seat seat;

    struct lw_surface_object* touch_obj;
    struct lw_surface_object* pointer_obj;
    struct lw_surface_object* keyboard_obj;

    lv_ll_t window_ll;
    lv_timer_t* cycle_timer;

    bool cursor_flush_pending;
};

struct _lw_window {
    lv_display_t* display;

    lv_indev_t* indev_pointer_p;
    lv_indev_t* indev_pointeraxis_p;
    lv_indev_t* indev_touch_p;
    lv_indev_t* indev_keyboard_p;

    lv_window_close_cb_t close_cb;

    struct lw_application* application;

#if LV_WAYLAND_XDG_SHELL
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    uint32_t wm_capabilities;
#endif

    struct lw_surface_object* body;
    struct {
        lv_area_t cache[LW_DMG_CACHE_CAPACITY];
        unsigned char start;
        unsigned char end;
        unsigned size;
    } dmg_cache;

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    struct lw_surface_object* decoration[LW_NUM_DECORATIONS];
#endif

    int width;
    int height;

    bool resize_pending;
    int resize_width;
    int resize_height;

    bool flush_pending;
    bool shall_close;
    bool closed;
    bool maximized;
    bool fullscreen;
    uint64_t frame_counter;
    bool frame_done;

    lv_display_render_mode_t render_mode;
};

/*********************************
 *   STATIC VARIABLES and FUNTIONS
 *********************************/

static struct lw_application application;

static const struct wl_callback_listener wl_surface_frame_listener;
static bool resize_window(lw_window_t* window, int width, int height);
static struct lw_surface_object* create_surface_obj(struct lw_application* app,
    lw_window_t* window,
    enum lw_object_type type,
    struct lw_surface_object* parent);

#if LV_WAYLAND_TICK_GET
static uint32_t tick_get_cb(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint64_t time_ms = t.tv_sec * 1000 + (t.tv_nsec / 1000000);
    return time_ms;
}
#endif

/**
 * @brief The frame callback called when the compositor has finished rendering
 * a frame.It increments the frame counter and sets up the callback
 * for the next frame the frame counter is used to avoid needlessly
 * committing frames too fast on a slow system
 *
 * NOTE: this function is invoked by the wayland-server library within the compositor
 * the event is added to the queue, and then upon the next timer call it's
 * called indirectly from handle_wayland_input (via wl_display_dispatch_queue)
 * @param void data the user object defined that was tied to this event during
 * the configuration of the callback
 * @param struct wl_callback The callback that needs to be destroyed and re-created
 * @param time Timestamp of the event (unused)
 */
static void frame_done_wl_cb(void* data, struct wl_callback* cb, uint32_t time)
{
    struct lw_surface_object* obj;
    lw_window_t* window;

    wl_callback_destroy(cb);

    obj = (struct lw_surface_object*)data;
    window = obj->window;
    window->frame_counter++;

    LV_LOG_TRACE("frame: %ld done, new frame: %ld",
        window->frame_counter - 1, window->frame_counter);

    window->frame_done = true;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = frame_done_wl_cb,
};

static void shm_format(void* data, struct wl_shm* wl_shm, uint32_t format)
{
    struct lw_application* app = data;

    switch (format) {
#if (LV_COLOR_DEPTH == 32)
    case WL_SHM_FORMAT_ARGB8888:
        app->format = format;
        break;
    case WL_SHM_FORMAT_XRGB8888:
        if (app->format != WL_SHM_FORMAT_ARGB8888) {
            app->format = format;
        }
        break;
#elif (LV_COLOR_DEPTH == 16)
    case WL_SHM_FORMAT_RGB565:
        app->format = format;
        break;
#elif (LV_COLOR_DEPTH == 8)
    case WL_SHM_FORMAT_RGB332:
        app->format = format;
        break;
#elif (LV_COLOR_DEPTH == 1)
    case WL_SHM_FORMAT_RGB332:
        app->format = format;
        break;
#endif
    default:
        break;
    }
}

static const struct wl_shm_listener shm_listener = { shm_format };

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
static bool attach_wl_decoration(lw_window_t* window, struct lw_surface_object* decoration,
    smm_buffer_t* decoration_buffer, struct lw_surface_object* parent)
{
    struct wl_buffer* wl_buf = SMM_BUFFER_PROPERTIES(decoration_buffer)->tag[LW_TAG_LOCAL];

    int pos_x, pos_y;

    switch (decoration->type) {
    case LW_OBJECT_TITLEBAR:
        pos_x = 0;
        pos_y = -LW_TITLE_BAR_HEIGHT;
        break;
    case LW_OBJECT_BUTTON_CLOSE:
        pos_x = parent->width - 1 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE);
        pos_y = -1 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE + (LW_BORDER_SIZE / 2));
        wl_surface_damage(decoration->surface, 0, 0, decoration->width, decoration->height);
        break;
#if LV_WAYLAND_XDG_SHELL
    case LW_OBJECT_BUTTON_MAXIMIZE:
        pos_x = parent->width - 2 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE);
        pos_y = -1 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE + (LW_BORDER_SIZE / 2));
        wl_surface_damage(decoration->surface, 0, 0, decoration->width, decoration->height);
        break;
    case LW_OBJECT_BUTTON_MINIMIZE:
        pos_x = parent->width - 3 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE);
        pos_y = -1 * (LW_BUTTON_MARGIN + LW_BUTTON_SIZE + (LW_BORDER_SIZE / 2));
        wl_surface_damage(decoration->surface, 0, 0, decoration->width, decoration->height);
        break;
#endif
    case LW_OBJECT_BORDER_TOP:
        pos_x = -LW_BORDER_SIZE;
        pos_y = -(LW_BORDER_SIZE + LW_TITLE_BAR_HEIGHT);
        break;
    case LW_OBJECT_BORDER_BOTTOM:
        pos_x = -LW_BORDER_SIZE;
        pos_y = parent->height;
        break;
    case LW_OBJECT_BORDER_LEFT:
        pos_x = -LW_BORDER_SIZE;
        pos_y = -LW_TITLE_BAR_HEIGHT;
        break;
    case LW_OBJECT_BORDER_RIGHT:
        pos_x = parent->width;
        pos_y = -LW_TITLE_BAR_HEIGHT;
        break;
    default:
        LV_ASSERT_MSG(0, "Invalid object type");
        return false;
    }

    if (decoration->subsurface == NULL) {
        /* Create the subsurface only once */
        decoration->subsurface = wl_subcompositor_get_subsurface(window->application->subcompositor,
            decoration->surface,
            parent->surface);
        if (!decoration->subsurface) {
            LV_LOG_ERROR("cannot get subsurface for decoration");
            goto err_destroy_surface;
        }
    }

    wl_subsurface_set_position(decoration->subsurface, pos_x, pos_y);
    wl_surface_attach(decoration->surface, wl_buf, 0, 0);
    wl_surface_commit(decoration->surface);

    return true;

err_destroy_surface:
    wl_surface_destroy(decoration->surface);
    decoration->surface = NULL;

    return false;
}

static void detach_wl_decoration(lw_window_t* window,
    struct lw_surface_object* decoration)
{
    if (decoration->subsurface) {
        wl_subsurface_destroy(decoration->subsurface);
        decoration->subsurface = NULL;
    }
}

static void fill_decoration_color(lv_color32_t* buf, lv_color32_t color, uint32_t px_num)
{
    while (px_num > 16) {
        *buf = color;  buf++; *buf = color; buf++; *buf = color; buf++; *buf = color; buf++;
        *buf = color;  buf++; *buf = color; buf++; *buf = color; buf++; *buf = color; buf++;
        *buf = color;  buf++; *buf = color; buf++; *buf = color; buf++; *buf = color; buf++;
        *buf = color;  buf++; *buf = color; buf++; *buf = color; buf++; *buf = color; buf++;

        px_num -= 16;
    }

    while (px_num) {
        *buf = color;
        buf++;
        px_num--;
    }
}

static bool create_wl_decoration(lw_window_t* window,
    struct lw_surface_object* decoration,
    int window_width, int window_height, uint8_t color_alpha)
{
    smm_buffer_t* buf;
    void* buf_base;
    int x, y;

    switch (decoration->type) {
    case LW_OBJECT_TITLEBAR:
        decoration->width = window_width;
        decoration->height = LW_TITLE_BAR_HEIGHT;
        break;
    case LW_OBJECT_BUTTON_CLOSE:
#if LV_WAYLAND_XDG_SHELL
    case LW_OBJECT_BUTTON_MAXIMIZE:
    case LW_OBJECT_BUTTON_MINIMIZE:
#endif
        decoration->width = LW_BUTTON_SIZE;
        decoration->height = LW_BUTTON_SIZE;
        break;
    case LW_OBJECT_BORDER_TOP:
    case LW_OBJECT_BORDER_BOTTOM:
        decoration->width = window_width + 2 * (LW_BORDER_SIZE);
        decoration->height = LW_BORDER_SIZE;
        break;
    case LW_OBJECT_BORDER_LEFT:
    case LW_OBJECT_BORDER_RIGHT:
        decoration->width = LW_BORDER_SIZE;
        decoration->height = window_height + LW_TITLE_BAR_HEIGHT;
        break;
    default:
        LV_ASSERT_MSG(0, "Invalid object type");
        return false;
    }

    smm_resize_group(decoration->buffer_group,
        (decoration->width * LW_COLOR_PX_SIZE) * decoration->height);

    buf = smm_acquire_buffer(decoration->buffer_group);
    if (buf == NULL) {
        LV_LOG_ERROR("cannot allocate buffer for decoration");
        return false;
    }

    buf_base = smm_map(buf);
    if (buf_base == NULL) {
        LV_LOG_ERROR("cannot map in allocated decoration buffer");
        smm_release_buffer(buf);
        return false;
    }

    switch (decoration->type) {
    case LW_OBJECT_TITLEBAR:
        fill_decoration_color((lv_color32_t*)buf_base, lv_color32_make(0x33, 0x33, 0x33, 0xff),
            (decoration->width * decoration->height));

        break;
    case LW_OBJECT_BUTTON_CLOSE:

        for (y = 0; y < decoration->height; y++) {
            for (x = 0; x < decoration->width; x++) {
                lv_color32_t* pixel = ((lv_color32_t*)buf_base + (y * decoration->width) + x);
                *pixel = lv_color32_make(color_alpha, 0x00, 0x00, color_alpha);

                if ((x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING)) {
                    if ((x == y) || (x == decoration->width - 1 - y)) {
                        *pixel = lv_color32_make(0xff, 0xff, 0xff, 0xff);
                    }
                    else if ((x == y - 1) || (x == decoration->width - y)) {
                        *pixel = lv_color32_make(0xff, 0xff, 0xff, 0xff);
                    }
                }
            }
        }
        break;
#if LV_WAYLAND_XDG_SHELL
    case LW_OBJECT_BUTTON_MAXIMIZE:
        for (y = 0; y < decoration->height; y++) {
            for (x = 0; x < decoration->width; x++) {
                lv_color32_t* pixel = ((lv_color32_t*)buf_base + (y * decoration->width) + x);
                *pixel = lv_color32_make(color_alpha, color_alpha, color_alpha, color_alpha);

                if (((x == LW_BUTTON_PADDING) && (y >= LW_BUTTON_PADDING) && (y < decoration->height - LW_BUTTON_PADDING)) ||
                    ((x == (decoration->width - LW_BUTTON_PADDING)) && (y >= LW_BUTTON_PADDING) && (y <= decoration->height - LW_BUTTON_PADDING)) ||
                    ((y == LW_BUTTON_PADDING) && (x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING)) ||
                    ((y == (LW_BUTTON_PADDING + 1)) && (x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING)) ||
                    ((y == (decoration->height - LW_BUTTON_PADDING)) && (x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING))) {
                    *pixel = lv_color32_make(0xff, 0xff, 0xff, 0xff);
                }

                if (window->maximized) {
                    if (((x == (decoration->width - LW_BUTTON_PADDING - 3)) && (y >= LW_BUTTON_PADDING + 3) && (y <= decoration->height - LW_BUTTON_PADDING)) ||
                        ((y == (LW_BUTTON_PADDING + 3)) && (x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING - 3))) {
                        *pixel = lv_color32_make(0xff, 0xff, 0xff, 0xff);
                    }
                }
            }
        }
        break;
    case LW_OBJECT_BUTTON_MINIMIZE:

        for (y = 0; y < decoration->height; y++) {
            for (x = 0; x < decoration->width; x++) {
                lv_color32_t* pixel = ((lv_color32_t*)buf_base + (y * decoration->width) + x);
                *pixel = lv_color32_make(color_alpha, color_alpha, color_alpha, color_alpha);

                if ((x >= LW_BUTTON_PADDING) && (x < decoration->width - LW_BUTTON_PADDING) &&
                    (y > decoration->height - (2 * LW_BUTTON_PADDING)) && (y < decoration->height - LW_BUTTON_PADDING)) {
                    *pixel = lv_color32_make(0xff, 0xff, 0xff, 0xff);
                }
            }
        }
        break;
#endif
    case LW_OBJECT_BORDER_TOP:
        /* fallthrough */
    case LW_OBJECT_BORDER_BOTTOM:
        /* fallthrough */
    case LW_OBJECT_BORDER_LEFT:
        /* fallthrough */
    case LW_OBJECT_BORDER_RIGHT:
        fill_decoration_color((lv_color32_t*)buf_base,
            lv_color32_make(0x33, 0x33, 0x33, 0xff), (decoration->width * decoration->height));
        break;
    default:
        LV_ASSERT_MSG(0, "Invalid object type");
        return false;
    }

    return attach_wl_decoration(window, decoration, buf, window->body);
}
#endif

static void handle_pointer_enter_wl_cb(void* data, struct wl_pointer* pointer,
    uint32_t serial, struct wl_surface* surface,
    wl_fixed_t sx, wl_fixed_t sy)
{
    struct lw_application* app = data;
    const char* cursor = "left_ptr";
    int pos_x = wl_fixed_to_int(sx);
    int pos_y = wl_fixed_to_int(sy);

    if (!surface) {
        app->pointer_obj = NULL;
        return;
    }

    app->pointer_obj = wl_surface_get_user_data(surface);

    app->pointer_obj->input.pointer.x = pos_x;
    app->pointer_obj->input.pointer.y = pos_y;

#if (LV_WAYLAND_CLIENT_SIDE_DECORATIONS && LV_WAYLAND_XDG_SHELL)
    if (!app->pointer_obj->window->xdg_toplevel || app->opt_disable_decorations) {
        return;
    }

    lw_window_t* window = app->pointer_obj->window;

    switch (app->pointer_obj->type) {
    case LW_OBJECT_BORDER_TOP:
        if (window->maximized) {
            // do nothing
        }
        else if (pos_x < (LW_BORDER_SIZE * 5)) {
            cursor = "top_left_corner";
        }
        else if (pos_x >= (window->width + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
            cursor = "top_right_corner";
        }
        else {
            cursor = "top_side";
        }
        break;
    case LW_OBJECT_BORDER_BOTTOM:
        if (window->maximized) {
            // do nothing
        }
        else if (pos_x < (LW_BORDER_SIZE * 5)) {
            cursor = "bottom_left_corner";
        }
        else if (pos_x >= (window->width + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
            cursor = "bottom_right_corner";
        }
        else {
            cursor = "bottom_side";
        }
        break;
    case LW_OBJECT_BORDER_LEFT:
        if (window->maximized) {
            // do nothing
        }
        else if (pos_y < (LW_BORDER_SIZE * 5)) {
            cursor = "top_left_corner";
        }
        else if (pos_y >= (window->height + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
            cursor = "bottom_left_corner";
        }
        else {
            cursor = "left_side";
        }
        break;
    case LW_OBJECT_BORDER_RIGHT:
        if (window->maximized) {
            // do nothing
        }
        else if (pos_y < (LW_BORDER_SIZE * 5)) {
            cursor = "top_right_corner";
        }
        else if (pos_y >= (window->height + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
            cursor = "bottom_right_corner";
        }
        else {
            cursor = "right_side";
        }
        break;
    case LW_OBJECT_BUTTON_CLOSE:
        // cursor = "bottom_right_corner";
        if (!create_wl_decoration(window, window->decoration[app->pointer_obj->type],
            window->body->width, window->body->height, 0x80)) {
            LV_LOG_ERROR("failed to create decoration %d", app->pointer_obj->type);
        }
        break;
    case LW_OBJECT_BUTTON_MAXIMIZE:
        // cursor = "top_right_corner";
    case LW_OBJECT_BUTTON_MINIMIZE:
        if (!create_wl_decoration(window, window->decoration[app->pointer_obj->type],
            window->body->width, window->body->height, 0x40)) {
            LV_LOG_ERROR("failed to create decoration %d", app->pointer_obj->type);
        }
        break;
    default:
        break;
    }
#endif

    if (app->cursor_surface) {
        struct wl_cursor_image* cursor_image = wl_cursor_theme_get_cursor(app->cursor_theme, cursor)->images[0];
        wl_pointer_set_cursor(pointer, serial, app->cursor_surface, cursor_image->hotspot_x, cursor_image->hotspot_y);
        wl_surface_attach(app->cursor_surface, wl_cursor_image_get_buffer(cursor_image), 0, 0);
        wl_surface_damage(app->cursor_surface, 0, 0, cursor_image->width, cursor_image->height);
        wl_surface_commit(app->cursor_surface);
        app->cursor_flush_pending = true;
    }
}

static void handle_pointer_leave_wl_cb(void* data, struct wl_pointer* pointer,
    uint32_t serial, struct wl_surface* surface)
{
    struct lw_application* app = data;

    if (!surface || (app->pointer_obj == wl_surface_get_user_data(surface))) {

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
        lw_window_t* window = app->pointer_obj->window;
        switch (app->pointer_obj->type) {
        case LW_OBJECT_BUTTON_CLOSE:
#if LV_WAYLAND_XDG_SHELL
        case LW_OBJECT_BUTTON_MAXIMIZE:
        case LW_OBJECT_BUTTON_MINIMIZE:
#endif
            if (!create_wl_decoration(window, window->decoration[app->pointer_obj->type],
                window->body->width, window->body->height, 0x00)) {
                LV_LOG_ERROR("failed to create decoration %d", app->pointer_obj->type);
            }
            break;
        default:
            break;
        }
        LV_LOG_INFO("type: %i, point-x:%i, y:%i\n", app->pointer_obj->type,
            app->pointer_obj->input.pointer.x, app->pointer_obj->input.pointer.y);
#endif

        app->pointer_obj = NULL;
    }
}

static void handle_pointer_motion_wl_cb(void* data, struct wl_pointer* pointer,
    uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    struct lw_application* app = data;

    if (!app->pointer_obj) {
        return;
    }

    app->pointer_obj->input.pointer.x = LV_MAX(0, LV_MIN(wl_fixed_to_int(sx), app->pointer_obj->width - 1));
    app->pointer_obj->input.pointer.y = LV_MAX(0, LV_MIN(wl_fixed_to_int(sy), app->pointer_obj->height - 1));
}

static void handle_pointer_button_wl_cb(void* data, struct wl_pointer* wl_pointer,
    uint32_t serial, uint32_t time, uint32_t button,
    uint32_t state)
{
    struct lw_application* app = data;
    const lv_indev_state_t lv_state =
        (state == WL_POINTER_BUTTON_STATE_PRESSED) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    if (!app->pointer_obj) {
        return;
    }

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    lw_window_t* window = app->pointer_obj->window;
    int pos_x = app->pointer_obj->input.pointer.x;
    int pos_y = app->pointer_obj->input.pointer.y;
#endif

    switch (app->pointer_obj->type) {
    case LW_OBJECT_WINDOW:
        switch (button) {
        case BTN_LEFT:
            app->pointer_obj->input.pointer.left_button = lv_state;
            break;
        case BTN_RIGHT:
            app->pointer_obj->input.pointer.right_button = lv_state;
            break;
        case BTN_MIDDLE:
            app->pointer_obj->input.pointer.wheel_button = lv_state;
            break;
        default:
            break;
        }
        break;
#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    case LW_OBJECT_TITLEBAR:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_PRESSED)) {
#if LV_WAYLAND_XDG_SHELL
            if (window->xdg_toplevel) {
                xdg_toplevel_move(window->xdg_toplevel, app->wl_seat, serial);
                window->flush_pending = true;
            }
#endif
        }
        break;
    case LW_OBJECT_BUTTON_CLOSE:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_RELEASED)) {
            window->shall_close = true;
            LV_LOG_INFO("CLOSE-button:%i\n", button);
        }
        break;
#if LV_WAYLAND_XDG_SHELL
    case LW_OBJECT_BUTTON_MAXIMIZE:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_RELEASED)) {
            if (window->xdg_toplevel) {
                if (window->maximized) {
                    xdg_toplevel_unset_maximized(window->xdg_toplevel);
                }
                else {
                    xdg_toplevel_set_maximized(window->xdg_toplevel);
                }
                window->maximized ^= true;
                window->flush_pending = true;
            }
        }
        break;
    case LW_OBJECT_BUTTON_MINIMIZE:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_RELEASED)) {
            if (window->xdg_toplevel) {
                xdg_toplevel_set_minimized(window->xdg_toplevel);
                window->flush_pending = true;
            }
        }
        break;
    case LW_OBJECT_BORDER_TOP:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_PRESSED)) {
            if (window->xdg_toplevel && !window->maximized) {
                uint32_t edge;
                if (pos_x < (LW_BORDER_SIZE * 5)) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                }
                else if (pos_x >= (window->width + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                }
                else {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                }
                xdg_toplevel_resize(window->xdg_toplevel,
                    window->application->wl_seat, serial, edge);
                window->flush_pending = true;
            }
        }
        break;
    case LW_OBJECT_BORDER_BOTTOM:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_PRESSED)) {
            if (window->xdg_toplevel && (!window->maximized)) {
                uint32_t edge;
                if (pos_x < (LW_BORDER_SIZE * 5)) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                }
                else if (pos_x >= (window->width + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                }
                else {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                }

                xdg_toplevel_resize(window->xdg_toplevel,
                    window->application->wl_seat, serial, edge);

                window->flush_pending = true;
            }
        }
        break;
    case LW_OBJECT_BORDER_LEFT:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_PRESSED)) {
            if (window->xdg_toplevel && (!window->maximized)) {
                uint32_t edge;
                if (pos_y < (LW_BORDER_SIZE * 5)) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                }
                else if (pos_y >= (window->height + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                }
                else {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                }
                xdg_toplevel_resize(window->xdg_toplevel,
                    window->application->wl_seat, serial, edge);
                window->flush_pending = true;
            }
        }
        break;
    case LW_OBJECT_BORDER_RIGHT:
        if ((button == BTN_LEFT) && (state == WL_POINTER_BUTTON_STATE_PRESSED)) {
            if (window->xdg_toplevel && (!window->maximized)) {
                uint32_t edge;
                if (pos_y < (LW_BORDER_SIZE * 5)) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                }
                else if (pos_y >= (window->height + LW_BORDER_SIZE - (LW_BORDER_SIZE * 5))) {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                }
                else {
                    edge = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                }
                xdg_toplevel_resize(window->xdg_toplevel,
                    window->application->wl_seat, serial, edge);
                window->flush_pending = true;
            }
        }
        break;
#endif // LV_WAYLAND_XDG_SHELL
#endif // LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    default:
        break;
    }
}

static void handle_pointer_axis_wl_cb(void* data, struct wl_pointer* wl_pointer,
    uint32_t time, uint32_t axis, wl_fixed_t value)
{
    struct lw_application* app = data;
    const int diff = wl_fixed_to_int(value);

    if (!app->pointer_obj) {
        return;
    }

    if (axis == 0) {
        if (diff > 0) {
            app->pointer_obj->input.pointer.wheel_diff++;
        }
        else if (diff < 0) {
            app->pointer_obj->input.pointer.wheel_diff--;
        }
    }
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = handle_pointer_enter_wl_cb,
    .leave = handle_pointer_leave_wl_cb,
    .motion = handle_pointer_motion_wl_cb,
    .button = handle_pointer_button_wl_cb,
    .axis = handle_pointer_axis_wl_cb,
};

static lv_key_t lw_keycode_xkb_to_lv(xkb_keysym_t xkb_key)
{
    lv_key_t key = 0;

    if (((xkb_key >= XKB_KEY_space) && (xkb_key <= XKB_KEY_asciitilde))) {
        key = xkb_key;
    }
    else if (((xkb_key >= XKB_KEY_KP_0) && (xkb_key <= XKB_KEY_KP_9))) {
        key = (xkb_key & 0x003f);
    }
    else {
        switch (xkb_key) {
        case XKB_KEY_BackSpace:
            key = LV_KEY_BACKSPACE;
            break;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
            key = LV_KEY_ENTER;
            break;
        case XKB_KEY_Escape:
            key = LV_KEY_ESC;
            break;
        case XKB_KEY_Delete:
        case XKB_KEY_KP_Delete:
            key = LV_KEY_DEL;
            break;
        case XKB_KEY_Home:
        case XKB_KEY_KP_Home:
            key = LV_KEY_HOME;
            break;
        case XKB_KEY_Left:
        case XKB_KEY_KP_Left:
            key = LV_KEY_LEFT;
            break;
        case XKB_KEY_Up:
        case XKB_KEY_KP_Up:
            key = LV_KEY_UP;
            break;
        case XKB_KEY_Right:
        case XKB_KEY_KP_Right:
            key = LV_KEY_RIGHT;
            break;
        case XKB_KEY_Down:
        case XKB_KEY_KP_Down:
            key = LV_KEY_DOWN;
            break;
        case XKB_KEY_Prior:
        case XKB_KEY_KP_Prior:
            key = LV_KEY_PREV;
            break;
        case XKB_KEY_Next:
        case XKB_KEY_KP_Next:
        case XKB_KEY_Tab:
        case XKB_KEY_KP_Tab:
            key = LV_KEY_NEXT;
            break;
        case XKB_KEY_End:
        case XKB_KEY_KP_End:
            key = LV_KEY_END;
            break;
        default:
            break;
        }
    }

    return key;
}

static void handle_keyboard_keymap_wl_cb(void* data, struct wl_keyboard* keyboard,
    uint32_t format, int fd, uint32_t size)
{
    struct lw_application* app = data;

    struct xkb_keymap* keymap;
    struct xkb_state* state;
    char* map_str;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    map_str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }

    /* Set up XKB keymap */
    keymap = xkb_keymap_new_from_string(app->xkb_context, map_str,
        XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    munmap(map_str, size);
    close(fd);

    if (!keymap) {
        LV_LOG_ERROR("failed to compile keymap");
        return;
    }

    /* Set up XKB state */
    state = xkb_state_new(keymap);
    if (!state) {
        LV_LOG_ERROR("failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    xkb_keymap_unref(app->seat.xkb.keymap);
    xkb_state_unref(app->seat.xkb.state);
    app->seat.xkb.keymap = keymap;
    app->seat.xkb.state = state;
}

static void handle_keyboard_enter_wl_cb(void* data, struct wl_keyboard* keyboard,
    uint32_t serial, struct wl_surface* surface,
    struct wl_array* keys)
{
    struct lw_application* app = data;

    if (!surface) {
        app->keyboard_obj = NULL;
    }
    else {
        app->keyboard_obj = wl_surface_get_user_data(surface);
    }
}

static void handle_keyboard_leave_wl_cb(void* data, struct wl_keyboard* keyboard,
    uint32_t serial, struct wl_surface* surface)
{
    struct lw_application* app = data;

    if (!surface || (app->keyboard_obj == wl_surface_get_user_data(surface))) {
        app->keyboard_obj = NULL;
    }
}

static void handle_keyboard_key_wl_cb(void* data, struct wl_keyboard* keyboard,
    uint32_t serial, uint32_t time, uint32_t key,
    uint32_t state)
{
    struct lw_application* app = data;
    const uint32_t code = (key + 8);
    const xkb_keysym_t* syms;
    xkb_keysym_t sym = XKB_KEY_NoSymbol;

    if (!app->keyboard_obj || !app->seat.xkb.state) {
        return;
    }

    if (xkb_state_key_get_syms(app->seat.xkb.state, code, &syms) == 1) {
        sym = syms[0];
    }

    const lv_key_t lv_key = lw_keycode_xkb_to_lv(sym);
    const lv_indev_state_t lv_state =
        (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    if (lv_key != 0) {
        app->keyboard_obj->input.keyboard.key = lv_key;
        app->keyboard_obj->input.keyboard.state = lv_state;
    }
}

static void handle_keyboard_modifiers_wl_cb(void* data, struct wl_keyboard* keyboard,
    uint32_t serial, uint32_t mods_depressed,
    uint32_t mods_latched, uint32_t mods_locked,
    uint32_t group)
{
    struct lw_application* app = data;

    /* If we're not using a keymap, then we don't handle PC-style modifiers */
    if (!app->seat.xkb.keymap) {
        return;
    }

    xkb_state_update_mask(app->seat.xkb.state,
        mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = handle_keyboard_keymap_wl_cb,
    .enter = handle_keyboard_enter_wl_cb,
    .leave = handle_keyboard_leave_wl_cb,
    .key = handle_keyboard_key_wl_cb,
    .modifiers = handle_keyboard_modifiers_wl_cb,
};

static void handle_touch_down_wl_cb(void* data, struct wl_touch* wl_touch,
    uint32_t serial, uint32_t time, struct wl_surface* surface,
    int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
    struct lw_application* app = data;

    if (!surface) {
        app->touch_obj = NULL;
        return;
    }

    app->touch_obj = wl_surface_get_user_data(surface);

    app->touch_obj->input.touch.x = wl_fixed_to_int(x_w);
    app->touch_obj->input.touch.y = wl_fixed_to_int(y_w);
    app->touch_obj->input.touch.state = LV_INDEV_STATE_PRESSED;

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    lw_window_t* window = app->touch_obj->window;
    switch (app->touch_obj->type) {
    case LW_OBJECT_TITLEBAR:
#if LV_WAYLAND_XDG_SHELL
        if (window->xdg_toplevel) {
            xdg_toplevel_move(window->xdg_toplevel, app->wl_seat, serial);
            window->flush_pending = true;
        }
#endif

        break;
    default:
        break;
    }
#endif
}

static void handle_touch_up_wl_cb(void* data, struct wl_touch* wl_touch,
    uint32_t serial, uint32_t time, int32_t id)
{
    struct lw_application* app = data;

    if (!app->touch_obj) {
        return;
    }

    app->touch_obj->input.touch.state = LV_INDEV_STATE_RELEASED;

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    lw_window_t* window = app->touch_obj->window;
    switch (app->touch_obj->type) {
    case LW_OBJECT_BUTTON_CLOSE:
        window->shall_close = true;
        break;
#if LV_WAYLAND_XDG_SHELL
    case LW_OBJECT_BUTTON_MAXIMIZE:
        if (window->xdg_toplevel) {
            if (window->maximized) {
                xdg_toplevel_unset_maximized(window->xdg_toplevel);
            }
            else {
                xdg_toplevel_set_maximized(window->xdg_toplevel);
            }
            window->maximized ^= true;
        }
        break;
    case LW_OBJECT_BUTTON_MINIMIZE:
        if (window->xdg_toplevel) {
            xdg_toplevel_set_minimized(window->xdg_toplevel);
            window->flush_pending = true;
        }
#endif // LV_WAYLAND_XDG_SHELL
    default:
        break;
    }
#endif // LV_WAYLAND_CLIENT_SIDE_DECORATIONS

    app->touch_obj = NULL;
}

static void handle_touch_motion_wl_cb(void* data, struct wl_touch* wl_touch,
    uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
    struct lw_application* app = data;

    if (!app->touch_obj) {
        return;
    }

    app->touch_obj->input.touch.x = wl_fixed_to_int(x_w);
    app->touch_obj->input.touch.y = wl_fixed_to_int(y_w);
}

static void handle_touch_frame_wl_cb(void* data, struct wl_touch* wl_touch)
{
}

static void handle_touch_cancel_wl_cb(void* data, struct wl_touch* wl_touch)
{
}

static const struct wl_touch_listener touch_listener = {
    .down = handle_touch_down_wl_cb,
    .up = handle_touch_up_wl_cb,
    .motion = handle_touch_motion_wl_cb,
    .frame = handle_touch_frame_wl_cb,
    .cancel = handle_touch_cancel_wl_cb,
};

static void handle_seat_capabilities_wl_cb(void* data, struct wl_seat* wl_seat, enum wl_seat_capability caps)
{
    struct lw_application* app = data;
    struct lw_seat* seat = &app->seat;

    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !seat->wl_pointer) {
        seat->wl_pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(seat->wl_pointer, &pointer_listener, app);
        app->cursor_surface = wl_compositor_create_surface(app->compositor);
        if (!app->cursor_surface) {
            LV_LOG_WARN("failed to create cursor surface");
        }
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && seat->wl_pointer) {
        wl_pointer_destroy(seat->wl_pointer);
        if (app->cursor_surface) {
            wl_surface_destroy(app->cursor_surface);
        }
        seat->wl_pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !seat->wl_keyboard) {
        seat->wl_keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(seat->wl_keyboard, &keyboard_listener, app);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && seat->wl_keyboard) {
        wl_keyboard_destroy(seat->wl_keyboard);
        seat->wl_keyboard = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !seat->wl_touch) {
        seat->wl_touch = wl_seat_get_touch(wl_seat);
        wl_touch_add_listener(seat->wl_touch, &touch_listener, app);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && seat->wl_touch) {
        wl_touch_destroy(seat->wl_touch);
        seat->wl_touch = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = handle_seat_capabilities_wl_cb,
};

static void draw_window(lw_window_t* window, uint32_t width, uint32_t height)
{

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    if (application.opt_disable_decorations == false) {
        int d;
        for (d = 0; d < LW_NUM_DECORATIONS; d++) {
            window->decoration[d] = create_surface_obj(&application, window, (LW_FIRST_DECORATION + d), window->body);
            if (!window->decoration[d]) {
                LV_LOG_ERROR("Failed to create decoration %d", d);
            }
        }
    }
#endif

    /* First resize */
    if (!resize_window(window, width, height)) {
        LV_LOG_ERROR("Failed to resize window");
#if LV_WAYLAND_XDG_SHELL
        if (window->xdg_toplevel) {
            xdg_toplevel_destroy(window->xdg_toplevel);
            window->xdg_toplevel = NULL;
        }
#endif
    }

    lv_refr_now(window->display);
}

#if LV_WAYLAND_XDG_SHELL
static void handle_xdg_surface_configure_wl_cb(void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
    lw_window_t* window = (lw_window_t*)data;
    // struct wl_buffer *wl_buf;

    xdg_surface_ack_configure(xdg_surface, serial);

    if (window->body->surface_configured == false) {
        draw_window(window, window->width, window->height);
    }

    window->body->surface_configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = handle_xdg_surface_configure_wl_cb,
};

static void handle_xdg_toplevel_configure_wl_cb(void* data, struct xdg_toplevel* xdg_toplevel,
    int32_t width, int32_t height, struct wl_array* states)
{
    lw_window_t* window = (lw_window_t*)data;
    int* sta = NULL;

    wl_array_for_each(sta, states)
    {
        if (sta) {
            LV_LOG_INFO("xdg sta:%i\n", *sta);
            if (*sta == XDG_TOPLEVEL_STATE_MAXIMIZED) {
                window->maximized = true;
                break;
            }
        }
    }

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    if (!window->application->opt_disable_decorations && !window->fullscreen) {
        if ((sta && *sta == XDG_TOPLEVEL_STATE_MAXIMIZED) ||
            (width && width < window->width) || (height && height < window->height)) {

            if (!create_wl_decoration(window, window->decoration[LW_OBJECT_BUTTON_MAXIMIZE],
                window->body->width, window->body->height, 0x00)) {
                LV_LOG_ERROR("config bound-failed to create decoration");
            }
            if (!(sta && *sta == XDG_TOPLEVEL_STATE_MAXIMIZED)) {
                window->maximized = false;
            }
        }

        width -= (2 * LW_BORDER_SIZE);
        height -= (LW_TITLE_BAR_HEIGHT + (2 * LW_BORDER_SIZE));
    }
#endif
    //ensure lv display buffer larger than the requirement, window will keep a minmized area.
    if ((width <= LW_BUTTON_SIZE * 10) || (height <= 4 * LW_TITLE_BAR_HEIGHT)) {
        return;
    }

    if ((width != window->width) || (height != window->height)) {
        window->resize_width = width;
        window->resize_height = height;
        window->resize_pending = true;
    }
}

static void handle_xdg_toplevel_close_wl_cb(void* data, struct xdg_toplevel* xdg_toplevel)
{
    lw_window_t* window = (lw_window_t*)data;
    window->shall_close = true;
}

static void handle_xdg_toplevel_configure_bounds_wl_cb(void* data, struct xdg_toplevel* xdg_toplevel,
    int32_t width, int32_t height)
{
    //lw_window_t *window = (lw_window_t *)data;

    /* Optional: Could set window width/height upper bounds, however, currently
     *           we'll honor the set width/height.
     */
}

static void handle_xdg_toplevel_wm_capabilities_wl_cb(void* data, struct xdg_toplevel* xdg_toplevel,
    struct wl_array* capabilities)
{
    uint32_t* cap;
    lw_window_t* window = (lw_window_t*)data;

    wl_array_for_each(cap, capabilities)
    {
        window->wm_capabilities |= (1 << (*cap));
        /* TODO: Disable appropriate graphics/capabilities as appropriate */
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = handle_xdg_toplevel_configure_wl_cb,
    .close = handle_xdg_toplevel_close_wl_cb,
    .configure_bounds = handle_xdg_toplevel_configure_bounds_wl_cb,
    .wm_capabilities = handle_xdg_toplevel_wm_capabilities_wl_cb /* This requires newer wayland headers */
};

static void xdg_wm_base_ping_cb(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial)
{
    return xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping_cb 
};
#endif

static void handle_global_wl_cb(void* data, struct wl_registry* registry,
    uint32_t name, const char* interface, uint32_t version)
{
    struct lw_application* app = data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        app->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        app->subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0) {
        app->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
        wl_shm_add_listener(app->shm, &shm_listener, app);
        app->cursor_theme = wl_cursor_theme_load(NULL, 32, app->shm);
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
        app->wl_seat = wl_registry_bind(app->registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(app->wl_seat, &seat_listener, app);
    }

#if LV_WAYLAND_XDG_SHELL
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        app->xdg_wm = wl_registry_bind(app->registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(app->xdg_wm, &xdg_wm_base_listener, app);
    }
#endif
}

static void handle_global_remove_wl_cb(void* data, struct wl_registry* registry, uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global_wl_cb,
    .global_remove = handle_global_remove_wl_cb };

static void handle_buffer_release_wl_cb(void* data, struct wl_buffer* wl_buffer)
{

    const struct smm_buffer_properties* props;
    struct lw_surface_object* obj;
    smm_buffer_t* buf;

    buf = (smm_buffer_t*)data;
    props = SMM_BUFFER_PROPERTIES(buf);
    obj = SMM_GROUP_PROPERTIES(props->group)->tag[LW_TAG_LOCAL];

    LV_LOG_TRACE("releasing buffer %p w:%d h:%d frame: %ld", wl_buffer, obj->width,
        obj->height, obj->window->frame_counter);
    (void)obj;
    smm_release_buffer((smm_buffer_t*)data);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = handle_buffer_release_wl_cb,
};

static void cache_clear(lw_window_t* window)
{
    window->dmg_cache.start = window->dmg_cache.end;
    window->dmg_cache.size = 0;
}

static void cache_purge(lw_window_t* window, smm_buffer_t* buf)
{
    lv_area_t* next_dmg;
    smm_buffer_t* next_buf = smm_next_buffer(buf);

    /* Remove all damage areas up until start of next buffers damage */
    if (next_buf == NULL) {
        cache_clear(window);
    }
    else {
        next_dmg = SMM_BUFFER_PROPERTIES(next_buf)->tag[LW_TAG_BUFFER_DAMAGE];
        while ((window->dmg_cache.cache + window->dmg_cache.start) != next_dmg) {
            window->dmg_cache.start++;
            window->dmg_cache.start %= LW_DMG_CACHE_CAPACITY;
            window->dmg_cache.size--;
        }
    }
}

static void cache_add_area(lw_window_t* window, smm_buffer_t* buf, const lv_area_t* area)
{
    if (SMM_BUFFER_PROPERTIES(buf)->tag[LW_TAG_BUFFER_DAMAGE] == NULL) {
        /* Buffer damage beyond cache capacity */
        return;
    }

    if ((window->dmg_cache.start == window->dmg_cache.end) &&
        (window->dmg_cache.size)) {
        /* This buffer has more damage then the cache's capacity, so
         * clear cache and leave buffer damage unrecorded
         */
        cache_clear(window);
        SMM_TAG(buf, LW_TAG_BUFFER_DAMAGE, NULL);
        return;
    }

    /* Add damage area to cache */
    lv_memcpy(window->dmg_cache.cache + window->dmg_cache.end,
        area,
        sizeof(lv_area_t));

    window->dmg_cache.end++;
    window->dmg_cache.end %= LW_DMG_CACHE_CAPACITY;
    window->dmg_cache.size++;
}

static void cache_apply_areas(lw_window_t* window, void* dest, void* src, smm_buffer_t* src_buf)
{
    unsigned long offset;
    unsigned char start;
    int y;
    lv_area_t* dmg;
    lv_area_t* next_dmg;
    smm_buffer_t* next_buf = smm_next_buffer(src_buf);
    const struct smm_buffer_properties* props = SMM_BUFFER_PROPERTIES(src_buf);
    struct lw_surface_object* obj = SMM_GROUP_PROPERTIES(props->group)->tag[LW_TAG_LOCAL];

    if (next_buf == NULL) {
        next_dmg = (window->dmg_cache.cache + window->dmg_cache.end);
    }
    else {
        next_dmg = SMM_BUFFER_PROPERTIES(next_buf)->tag[LW_TAG_BUFFER_DAMAGE];
    }

    /* Apply all buffer damage areas */
    start = ((lv_area_t*)SMM_BUFFER_PROPERTIES(src_buf)->tag[LW_TAG_BUFFER_DAMAGE] - window->dmg_cache.cache);
    while ((window->dmg_cache.cache + start) != next_dmg) {
        /* Copy an area from source to destination (line-by-line) */
        dmg = (window->dmg_cache.cache + start);
        for (y = dmg->y1; y <= dmg->y2; y++) {
            offset = (dmg->x1 + (y * (obj->width * LW_COLOR_PX_SIZE)));
            lv_memcpy(((char*)dest) + offset,
                ((char*)src) + offset,
                ((dmg->x2 - dmg->x1 + 1) * LW_COLOR_PX_SIZE));
        }

        start++;
        start %= LW_DMG_CACHE_CAPACITY;
    }
}

static bool sme_new_pool(void* ctx, smm_pool_t* pool)
{
    struct wl_shm_pool* wl_pool;
    struct lw_application* app = ctx;
    const struct smm_pool_properties* props = SMM_POOL_PROPERTIES(pool);

    wl_pool = wl_shm_create_pool(app->shm,
        props->fd,
        props->size);

    SMM_TAG(pool, LW_TAG_LOCAL, wl_pool);
    return (wl_pool == NULL);
}

static void sme_expand_pool(void* ctx, smm_pool_t* pool)
{
    const struct smm_pool_properties* props = SMM_POOL_PROPERTIES(pool);

    wl_shm_pool_resize(props->tag[LW_TAG_LOCAL], props->size);
}

static void sme_free_pool(void* ctx, smm_pool_t* pool)
{
    struct wl_shm_pool* wl_pool = SMM_POOL_PROPERTIES(pool)->tag[LW_TAG_LOCAL];
    wl_shm_pool_destroy(wl_pool);
}

static bool sme_new_buffer(void* ctx, smm_buffer_t* buf)
{
    struct wl_buffer* wl_buf;
    bool fail_alloc = true;
    const struct smm_buffer_properties* props = SMM_BUFFER_PROPERTIES(buf);
    struct wl_shm_pool* wl_pool = SMM_POOL_PROPERTIES(props->pool)->tag[LW_TAG_LOCAL];
    struct lw_application* app = ctx;
    struct lw_surface_object* obj = SMM_GROUP_PROPERTIES(props->group)->tag[LW_TAG_LOCAL];

    wl_buf = wl_shm_pool_create_buffer(wl_pool,
        props->offset,
        obj->width,
        obj->height,
        obj->width * LW_COLOR_PX_SIZE,
        app->format);

    if (wl_buf != NULL) {
        wl_buffer_add_listener(wl_buf, &wl_buffer_listener, buf);
        SMM_TAG(buf, LW_TAG_LOCAL, wl_buf);
        SMM_TAG(buf, LW_TAG_BUFFER_DAMAGE, NULL);
        fail_alloc = false;
    }

    return fail_alloc;
}

static bool sme_init_buffer(void* ctx, smm_buffer_t* buf)
{
    smm_buffer_t* src;
    void* src_base;
    bool fail_init = true;
    bool dmg_missing = false;
    void* buf_base = smm_map(buf);

    const struct smm_buffer_properties* props = SMM_BUFFER_PROPERTIES(buf);
    struct lw_surface_object* obj = SMM_GROUP_PROPERTIES(props->group)->tag[LW_TAG_LOCAL];

    if (buf_base == NULL) {
        LV_LOG_ERROR("cannot map in buffer to initialize");
        goto done;
    }

    /* Determine if all subsequent buffers damage is recorded */
    for (src = smm_next_buffer(buf); src != NULL; src = smm_next_buffer(src)) {
        if (SMM_BUFFER_PROPERTIES(src)->tag[LW_TAG_BUFFER_DAMAGE] == NULL) {
            dmg_missing = true;
            break;
        }
    }

    if ((smm_next_buffer(buf) == NULL) || dmg_missing) {
        /* Missing subsequent buffer damage, initialize by copying the most
         * recently acquired buffers data
         */
        src = smm_latest_buffer(props->group);
        if ((src != NULL) &&
            (src != buf)) {
            /* Map and copy latest buffer data */
            src_base = smm_map(src);
            if (src_base == NULL) {
                LV_LOG_ERROR("cannot map most recent buffer to copy");
                goto done;
            }

            lv_memcpy(buf_base,
                src_base,
                (obj->width * LW_COLOR_PX_SIZE) * obj->height);
        }
    }
    else {
        /* All subsequent buffers damage is recorded, initialize by applying
         * their damage to this buffer
         */
        for (src = smm_next_buffer(buf); src != NULL; src = smm_next_buffer(src)) {
            src_base = smm_map(src);
            if (src_base == NULL) {
                LV_LOG_ERROR("cannot map source buffer to copy from");
                goto done;
            }

            cache_apply_areas(obj->window, buf_base, src_base, src);
        }

        /* Purge out-of-date cached damage (up to and including next buffer) */
        src = smm_next_buffer(buf);
        if (src == NULL) {
            cache_purge(obj->window, src);
        }
    }

    fail_init = false;
done:
    return fail_init;
}

static void sme_free_buffer(void* ctx, smm_buffer_t* buf)
{
    struct wl_buffer* wl_buf = SMM_BUFFER_PROPERTIES(buf)->tag[LW_TAG_LOCAL];
    wl_buffer_destroy(wl_buf);
}

static struct lw_surface_object* create_surface_obj(struct lw_application* app, lw_window_t* window,
    enum lw_object_type type,
    struct lw_surface_object* parent)
{
    struct lw_surface_object* obj = malloc(sizeof(*obj));
    LV_ASSERT_MALLOC(obj);
    if (!obj) {
        goto err_out;
    }

    memset(obj, 0x00, sizeof(struct lw_surface_object));

    obj->surface = wl_compositor_create_surface(app->compositor);
    if (!obj->surface) {
        LV_LOG_ERROR("cannot create surface for graphic object");
        goto err_free;
    }

    obj->buffer_group = smm_create_group();
    if (obj->buffer_group == NULL) {
        LV_LOG_ERROR("cannot create buffer group for graphic object");
        goto err_destroy_surface;
    }

    obj->window = window;
    obj->type = type;
    obj->surface_configured = true;
    obj->pending_buffer = NULL;
    wl_surface_set_user_data(obj->surface, obj);
    SMM_TAG(obj->buffer_group, LW_TAG_LOCAL, obj);

    return obj;

err_destroy_surface:
    wl_surface_destroy(obj->surface);

err_free:
    free(obj);
err_out:

    return NULL;
}

static void destroy_wl_surface_obj(struct lw_surface_object* obj)
{
    if (obj->subsurface) {
        wl_subsurface_destroy(obj->subsurface);
    }

    wl_surface_destroy(obj->surface);
    smm_destroy_group(obj->buffer_group);
    free(obj);
}

static bool resize_window(lw_window_t* window, int width, int height)
{
    lv_color32_t* buf1 = NULL;
    struct smm_buffer_t* body_buf1;
    //struct smm_buffer_t* body_buf2;
    int b;

    LV_LOG_TRACE("resize window %dx%d frame: %ld rendered: %d",
        width, height, window->frame_counter, window->frame_done);

/* Update size for newly allocated buffers */
    smm_resize_group(window->body->buffer_group, ((width * LW_COLOR_PX_SIZE) * height) * 2);

    window->width = width;
    window->height = height;

    window->body->width = width;
    window->body->height = height;

    /* Pre-allocate two buffers for the window body here */
    body_buf1 = smm_acquire_buffer(window->body->buffer_group);
    /*body_buf2 = smm_acquire_buffer(window->body->buffer_group);

    if (smm_map(body_buf2) == NULL) {
        LV_LOG_ERROR("Cannot pre-allocate backing buffers for window body");
        wl_surface_destroy(window->body->surface);
        return false;
    }
*/
    /* Moves the buffers to the the unused list of the group */
    smm_release_buffer(body_buf1);
    //smm_release_buffer(body_buf2);

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    if (!window->application->opt_disable_decorations && !window->fullscreen) {
        for (b = 0; b < LW_NUM_DECORATIONS; b++) {
            if (!create_wl_decoration(window, window->decoration[b],
                window->body->width, window->body->height, 0x00)) {
                LV_LOG_ERROR("failed to create decoration %d", b);
            }
        }
    }
    else if (!window->application->opt_disable_decorations) {
        /* Entering fullscreen, detach decorations to prevent xdg_wm_base error 4 */
        /* requested geometry larger than the configured fullscreen state */
        for (b = 0; b < LW_NUM_DECORATIONS; b++) {
            detach_wl_decoration(window, window->decoration[b]);
        }
    }
#endif

    if (window->display != NULL) {
        /* Resize draw buffer */
        uint32_t data_size = ((width * height) / LW_DRAW_BUFFER_DIV) * sizeof(lv_color32_t);
        buf1 = malloc(data_size);
        if (!buf1) {
            LV_LOG_ERROR("failed to resize draw buffer");
            return false;
        }

        lv_draw_buf_t* db = lv_display_get_buf_active(window->display);
        if (db->unaligned_data) {
            free(db->unaligned_data);
        }

        lv_display_set_buffers(window->display, buf1, NULL, data_size, window->render_mode);
        /* Propagate resize to LVGL */
        lv_display_set_resolution(window->display, width, height);

        window->body->input.pointer.x = LV_MIN(window->body->input.pointer.x, (width - 1));
        window->body->input.pointer.y = LV_MIN(window->body->input.pointer.y, (height - 1));
    }

    return true;
}

static lw_window_t* create_wl_window(struct lw_application* app, int width, int height, const char* title)
{
    lw_window_t* window = _lv_ll_ins_tail(&app->window_ll);
    LV_ASSERT_MALLOC(window);
    if (!window) {
        return NULL;
    }

    memset(window, 0x00, sizeof(lw_window_t));

    window->application = app;
    window->width = width;
    window->height = height;
    // Create wayland buffer and surface
    window->body = create_surface_obj(app, window, LW_OBJECT_WINDOW, NULL);
    if (!window->body) {
        LV_LOG_ERROR("cannot create window body");
        goto err_free_window;
    }

    // Create shell surface
#if LV_WAYLAND_XDG_SHELL
    if (app->xdg_wm) {
        window->xdg_surface = xdg_wm_base_get_xdg_surface(app->xdg_wm, window->body->surface);
        if (!window->xdg_surface) {
            LV_LOG_ERROR("cannot create XDG surface");
            goto err_destroy_surface;
        }

        xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

        window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
        if (!window->xdg_toplevel) {
            LV_LOG_ERROR("cannot get XDG toplevel surface");
            goto err_destroy_shell_surface;
        }

        xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

        xdg_toplevel_set_title(window->xdg_toplevel, title);
        xdg_toplevel_set_app_id(window->xdg_toplevel, title);

        // XDG surfaces need to be configured before a buffer can be attached.
        // An (XDG) surface commit (without an attached buffer) triggers this
        // configure event
        window->body->surface_configured = false;
        wl_surface_commit(window->body->surface);
        /* Manually dispatching the queue is required to */
        /* ensure that configure gets called right away. */
        wl_display_dispatch(application.wl_disp);
    }
#endif

    else {
        LV_LOG_ERROR("No shell available");
        goto err_destroy_surface;
    }

    return window;

err_destroy_shell_surface:

#if LV_WAYLAND_XDG_SHELL
    if (window->xdg_surface) {
        xdg_surface_destroy(window->xdg_surface);
    }
#endif

err_destroy_surface:
    wl_surface_destroy(window->body->surface);

err_free_window:
    _lv_ll_remove(&app->window_ll, window);
    free(window);
    return NULL;
}

static void destroy_wl_window(lw_window_t* window)
{
    if (!window) {
        return;
    }

#if LV_WAYLAND_XDG_SHELL
    if (window->xdg_toplevel) {
        xdg_toplevel_destroy(window->xdg_toplevel);
        xdg_surface_destroy(window->xdg_surface);
    }
#endif

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    int b;
    for (b = 0; b < LW_NUM_DECORATIONS; b++) {
        if (window->decoration[b]) {
            destroy_wl_surface_obj(window->decoration[b]);
            window->decoration[b] = NULL;
        }
    }
#endif

    destroy_wl_surface_obj(window->body);
}

static void flush_lv_cb(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
    unsigned long offset = 0;

    int32_t y;
    void* buf_base;
    struct wl_buffer* wl_buf;
    int src_width = lv_area_get_width(area);
    int src_height = lv_area_get_height(area);
    lw_window_t* window = lv_display_get_user_data(display);
    smm_buffer_t* buf = window->body->pending_buffer;
    struct wl_callback* cb;

    const int hres = (lv_display_get_rotation(display) == 0) ? lv_display_get_horizontal_resolution(display) : lv_display_get_vertical_resolution(display);
    const int vres = (lv_display_get_rotation(display) == 0) ? lv_display_get_vertical_resolution(display) : lv_display_get_horizontal_resolution(display);

    /* If window has been / is being closed, or is not visible, skip flush */
    if (window->closed || window->shall_close) {
        goto skip;
    }
    /* Skip if the area is out the screen */
    else if ((area->x2 < 0) || (area->y2 < 0) || (area->x1 > hres - 1) || (area->y1 > vres - 1)) {
        goto skip;
    }
    /* Acquire and map a buffer to attach/commit to surface */
    if (buf == NULL) {
        buf = smm_acquire_buffer(window->body->buffer_group);
        if (buf == NULL) {
            LV_LOG_ERROR("cannot acquire a window body buffer");
            goto skip;
        }

        window->body->pending_buffer = buf;
        SMM_TAG(buf,
            LW_TAG_BUFFER_DAMAGE,
            window->dmg_cache.cache + window->dmg_cache.end);
    }

    buf_base = smm_map(buf);
    if (buf_base == NULL) {
        LV_LOG_ERROR("cannot map in window body buffer");
        goto skip;
    }

    if (window->render_mode == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        /* Modify specified area in buffer */
        lv_color_format_t cf = lv_display_get_color_format(display);
        uint32_t px_map_stride = lv_draw_buf_width_to_stride(src_width, cf);
        uint32_t px_map_line_bytes = src_width * LW_COLOR_PX_SIZE;
        int32_t hor_res = lv_display_get_horizontal_resolution(display);

        for (y = area->y1; y <= area->y2; y++) {
            offset = ((area->x1 + (y * hor_res)) * LW_COLOR_PX_SIZE);

            lv_memcpy(((char*)buf_base) + offset, px_map, px_map_line_bytes);
            px_map += px_map_stride;
        }
    }

    /* Mark surface damage */
    wl_surface_damage(window->body->surface,
        area->x1,
        area->y1,
        src_width,
        src_height);

    /* Cache buffer damage for future buffer initializations */
    cache_add_area(window, buf, area);

    if (lv_display_flush_is_last(display)) {

        /* Finally, attach buffer and commit to surface */
        wl_buf = SMM_BUFFER_PROPERTIES(buf)->tag[LW_TAG_LOCAL];
        wl_surface_attach(window->body->surface, wl_buf, 0, 0);
        wl_surface_commit(window->body->surface);
        window->body->pending_buffer = NULL;
        window->frame_done = false;

        cb = wl_surface_frame(window->body->surface);
        wl_callback_add_listener(cb, &wl_surface_frame_listener, window->body);
        LV_LOG_TRACE("last flush frame: %ld", window->frame_counter);
        window->flush_pending = true;
    }

    lv_display_flush_ready(display);
    return;

skip:
    if (buf != NULL) {
        /* Cleanup any intermediate state (in the event that this flush being
         * skipped is in the middle of a flush sequence)
         */
        cache_clear(window);
        SMM_TAG(buf, LW_TAG_BUFFER_DAMAGE, NULL);
        smm_release_buffer(buf);
        window->body->pending_buffer = NULL;
    }
}

static void handle_wayland_input(void)
{
    while (wl_display_prepare_read(application.wl_disp) != 0) {
        wl_display_dispatch_pending(application.wl_disp);
    }

    wl_display_read_events(application.wl_disp);
    LV_LOG_INFO("wl event\n");

    wl_display_dispatch_pending(application.wl_disp);
}

static void handle_wayland_output(void)
{
    lw_window_t* window;
    bool shall_flush = application.cursor_flush_pending;

    _LV_LL_READ(&application.window_ll, window)
    {
        if ((window->shall_close) && (window->close_cb != NULL)) {
            window->shall_close = window->close_cb(window->display);
            if(window->shall_close){
                window->close_cb = NULL;
            }
        }

        if (window->closed) {
            continue;
        }
        else if (window->shall_close) {
            window->closed = true;
            window->shall_close = false;
            shall_flush = true;

            window->body->input.touch.x = 0;
            window->body->input.touch.y = 0;
            window->body->input.touch.state = LV_INDEV_STATE_RELEASED;
            if (window->application->touch_obj == window->body) {
                window->application->touch_obj = NULL;
            }

            window->body->input.pointer.x = 0;
            window->body->input.pointer.y = 0;
            window->body->input.pointer.left_button = LV_INDEV_STATE_RELEASED;
            window->body->input.pointer.right_button = LV_INDEV_STATE_RELEASED;
            window->body->input.pointer.wheel_button = LV_INDEV_STATE_RELEASED;
            window->body->input.pointer.wheel_diff = 0;
            if (window->application->pointer_obj == window->body) {
                window->application->pointer_obj = NULL;
            }

            window->body->input.keyboard.key = 0;
            window->body->input.keyboard.state = LV_INDEV_STATE_RELEASED;
            if (window->application->keyboard_obj == window->body) {
                window->application->keyboard_obj = NULL;
            }
            destroy_wl_window(window);
        }

        shall_flush |= window->flush_pending;
    }

    if (shall_flush) {
        if (wl_display_flush(application.wl_disp) == -1) {
            if (errno != EAGAIN) {
                LV_LOG_ERROR("failed to flush wayland display");
            }
        }
        else {
            /* All data flushed */
            application.cursor_flush_pending = false;
            _LV_LL_READ(&application.window_ll, window)
            {
                window->flush_pending = false;
            }
        }
    }
}

static void handle_io_lv_cb(lv_timer_t* tmr)
{
    lw_window_t* window;
    lv_timer_t* input_timer[4];

    LV_UNUSED(tmr);

    handle_wayland_input();

    /* Ready input timers (to probe for any input recieved) */
    _LV_LL_READ(&application.window_ll, window)
    {
        LV_LOG_TRACE("handle timer frame: %ld", window->frame_counter);

        if (window != NULL && window->frame_done == false && window->frame_counter > 0) {
            LV_LOG_TRACE("LVGL is going too fast or window is hidden");
            continue;
        }
        else if (window != NULL &&
            window->resize_pending && window->frame_counter > 0) {
            if (resize_window(window, window->resize_width, window->resize_height)) {
                window->resize_width = window->width;
                window->resize_height = window->height;
                window->resize_pending = false;
            }
            else {
                LV_LOG_TRACE("Failed to resize window frame: %ld", window->frame_counter);
            }
        }
        else {
            continue;
        }

        input_timer[0] = lv_indev_get_read_timer(window->indev_pointer_p);
        input_timer[1] = lv_indev_get_read_timer(window->indev_pointeraxis_p);
        input_timer[2] = lv_indev_get_read_timer(window->indev_keyboard_p);
        input_timer[3] = lv_indev_get_read_timer(window->indev_touch_p);

        for (uint8_t i = 0; i < 4; i++) {
            if (input_timer[i]) {
                lv_timer_ready(input_timer[i]);
            }
        }
    }

    handle_wayland_output();
}

static void read_pointer_lv_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    lw_window_t* window = lv_display_get_user_data(lv_indev_get_display(indev));
    ;
    if (!window || window->closed) {
        return;
    }

    data->point.x = window->body->input.pointer.x;
    data->point.y = window->body->input.pointer.y;
    data->state = window->body->input.pointer.left_button;
}

static void read_pointeraxis_lv_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    lw_window_t* window = lv_display_get_user_data(lv_indev_get_display(indev));
    ;
    if (!window || window->closed) {
        return;
    }

    data->state = window->body->input.pointer.wheel_button;
    data->enc_diff = window->body->input.pointer.wheel_diff;

    window->body->input.pointer.wheel_diff = 0;
}

static void read_keyboard_lv_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    lw_window_t* window = lv_display_get_user_data(lv_indev_get_display(indev));
    ;
    if (!window || window->closed) {
        return;
    }

    data->key = window->body->input.keyboard.key;
    data->state = window->body->input.keyboard.state;
}

static void read_touch_lv_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    lw_window_t* window = lv_display_get_user_data(lv_indev_get_display(indev));
    if (!window || window->closed) {
        return;
    }

    data->point.x = window->body->input.touch.x;
    data->point.y = window->body->input.touch.y;
    data->state = window->body->input.touch.state;
}

static void create_lv_input_devices(lw_window_t* window)
{
    /* for keyboard */
    lv_group_t* group = lv_group_get_default();
    if (!group) {
        group = lv_group_create();
        if (group) {
            lv_group_set_default(group);
        }
    }
    /* Register input */
    window->indev_pointer_p = lv_indev_create();
    if (window->indev_pointer_p) {
        lv_indev_set_type(window->indev_pointer_p, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(window->indev_pointer_p, read_pointer_lv_cb);
        lv_indev_set_display(window->indev_pointer_p, window->display);

        lv_indev_set_group(window->indev_pointer_p, group);
    }
    else {
        LV_LOG_ERROR("failed to register pointer indev");
    }

    window->indev_pointeraxis_p = lv_indev_create();
    if (window->indev_pointeraxis_p) {
        lv_indev_set_type(window->indev_pointeraxis_p, LV_INDEV_TYPE_ENCODER);
        lv_indev_set_read_cb(window->indev_pointeraxis_p, read_pointeraxis_lv_cb);
        lv_indev_set_display(window->indev_pointeraxis_p, window->display);

        //lv_indev_set_group(window->indev_pointeraxis_p, group);
    }
    else {
        LV_LOG_ERROR("failed to register pointeraxis indev");
    }

    window->indev_touch_p = lv_indev_create();
    if (window->indev_touch_p) {
        lv_indev_set_type(window->indev_touch_p, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(window->indev_touch_p, read_touch_lv_cb);
        lv_indev_set_display(window->indev_touch_p, window->display);
    }
    else {
        LV_LOG_ERROR("failed to register touch indev");
    }

    window->indev_keyboard_p = lv_indev_create();
    if (window->indev_keyboard_p) {
        lv_indev_set_type(window->indev_keyboard_p, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(window->indev_keyboard_p, read_keyboard_lv_cb);
        lv_indev_set_display(window->indev_keyboard_p, window->display);
        /* the keyboard must be set to the group.*/
        lv_indev_set_group(window->indev_keyboard_p, group);
    }
    else {
        LV_LOG_ERROR("failed to register keyboard indev");
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Get Wayland display file descriptor
 * @return Wayland display file descriptor
 */
int lv_get_wayland_fd(void)
{
    return wl_display_get_fd(application.wl_disp);
}

/**
 * Close wayland window
 * @param disp LVGL display using window to be closed
 */
bool lv_close_wayland_window(lv_display_t* disp)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if ((!window) || window->closed) {
        return false;
    }
    window->shall_close = true;
    window->close_cb = NULL;

    return true;
}

/**
 * Check if a Wayland window is open on the specified display. Otherwise (if
 * argument is NULL), check if any Wayland window is open.
 * @return true if window open, false otherwise
 */
bool lv_is_window_open(lv_display_t* disp)
{
    lw_window_t* window;
    bool open = false;

    if (disp == NULL) {
        _LV_LL_READ(&application.window_ll, window)
        {
            if (!window->closed) {
                open = true;
                break;
            }
        }
    }
    else {
        window = lv_display_get_user_data(disp);

        open = window ? (!window->closed) : false;
    }

    return open;
}

/**
 * Set/unset window fullscreen mode
 * @param disp LVGL display using window to be set/unset fullscreen
 * @param fullscreen requested status (true = fullscreen)
 */
void lv_set_wayland_window_fullscreen(lv_display_t* disp, bool fullscreen)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if (!window || window->closed) {
        return;
    }

    if (window->fullscreen != fullscreen) {

#if LV_WAYLAND_XDG_SHELL
        if (window->xdg_toplevel) {
            if (fullscreen) {
                xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
            }
            else {
                xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
            }
            window->fullscreen = fullscreen;
            window->flush_pending = true;

            return;
        }
#endif

        else {
            LV_LOG_WARN("Wayland fullscreen mode not supported");
        }
    }
}

/**
 * Get pointer input device for given LVGL display
 * @param disp LVGL display
 * @return input device connected to pointer events, or NULL on error
 */
lv_indev_t* lv_get_wayland_pointer(lv_display_t* disp)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if (!window) {
        return NULL;
    }
    return window->indev_pointer_p;
}

/**
 * Get pointer axis input device for given LVGL display
 * @param disp LVGL display
 * @return input device connected to pointer axis events, or NULL on error
 */
lv_indev_t* lv_get_wayland_pointeraxis(lv_display_t* disp)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if (!window) {
        return NULL;
    }
    return window->indev_pointeraxis_p;
}

/**
 * Get keyboard input device for given LVGL display
 * @param disp LVGL display
 * @return input device connected to keyboard, or NULL on error
 */
lv_indev_t* lv_get_wayland_keyboard(lv_display_t* disp)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if (!window) {
        return NULL;
    }
    return window->indev_keyboard_p;
}

/**
 * Get touchscreen input device for given LVGL display
 * @param disp LVGL display
 * @return input device connected to touchscreen, or NULL on error
 */
lv_indev_t* lv_get_wayland_touchscreen(lv_display_t* disp)
{
    lw_window_t* window = lv_display_get_user_data(disp);
    if (!window) {
        return NULL;
    }
    return window->indev_touch_p;
}

/**
 * Wayland specific timer handler (use in place of LVGL lv_timer_handler)
 * @return time until next timer expiry in milliseconds
 */
uint32_t lv_wayland_timer_handler(void)
{
    lw_window_t* window;
    uint32_t time_until_next = lv_timer_get_time_until_next();
    /* LVGL handling */
    lv_timer_periodic_handler();

    /* Set 'errno' if a Wayland flush is outstanding (i.e. data still needs to
     * be sent to the compositor, but the compositor pipe/connection is unable
     * to take more data at this time).
     */
    _LV_LL_READ(&application.window_ll, window)
    {
        if (window->flush_pending) {
            errno = EAGAIN;
            break;
        }
    }

    return (!lv_is_window_open(NULL)) ? LV_NO_TIMER_READY : time_until_next;
}

/**
 * Create wayland window
 * @param hor_res initial horizontal window size in pixels
 * @param ver_res initial vertical window size in pixels
 * @param title window title
 * @param close_cb function to be called when the window gets closed by the user (optional)
 * @return new display backed by a Wayland window, or NULL on error
 */
lv_display_t* lv_create_wayland_window(int hor_res, int ver_res, char* title,
    lv_window_close_cb_t close_cb)
{

    lw_window_t* window = create_wl_window(&application, hor_res, ver_res, title);
    if (!window) {
        LV_LOG_ERROR("failed to create wayland window");
        return NULL;
    }

    window->close_cb = close_cb;

    /* Initialize display driver */
    window->display = lv_display_create(hor_res, ver_res);
    if (!window->display) {
        LV_LOG_ERROR("failed to crating display");
        destroy_wl_window(window);
        return NULL;
    }
    create_lv_input_devices(window);

    /* Initialize draw buffer */
    uint32_t data_size = ((hor_res * ver_res) / LW_DRAW_BUFFER_DIV) * sizeof(lv_color32_t);
    void* buf1 = malloc(data_size);
    if (!buf1) {
        LV_LOG_ERROR("failed to allocate draw buffer");
        destroy_wl_window(window);
        return NULL;
    }

    lv_display_set_color_format(window->display, LV_COLOR_FORMAT_XRGB8888);
    if (LW_COLOR_PX_SIZE != lv_color_format_get_size(lv_display_get_color_format(window->display))) {
        LV_LOG_ERROR("Corlor format not right");
        return NULL;
    }

    window->render_mode = LV_DISPLAY_RENDER_MODE_PARTIAL;
    lv_display_set_buffers(window->display, buf1, NULL, data_size, window->render_mode);
    lv_display_set_flush_cb(window->display, flush_lv_cb);
    lv_display_set_resolution(window->display, hor_res, ver_res);

    lv_display_set_user_data(window->display, window);

    return window->display;
}

/**
 * Initialize Wayland driver
 */

bool lv_wayland_init(void)
{
#if LV_WAYLAND_TICK_GET
    lv_tick_set_cb(tick_get_cb);
#endif

    if (lv_strcmp(getenv("XDG_SESSION_TYPE"), "wayland") != 0) {
        LV_LOG_ERROR("No wayland applicated!");
        return false;
    }

    struct smm_events evs = {
        NULL,
        sme_new_pool,
        sme_expand_pool,
        sme_free_pool,
        sme_new_buffer,
        sme_init_buffer,
        sme_free_buffer };

    LV_ASSERT_MSG(getenv("XDG_RUNTIME_DIR"), "cannot get XDG_RUNTIME_DIR");

    // Create XKB context
    application.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    LV_ASSERT_MSG(application.xkb_context, "failed to create XKB context");

    // Connect to Wayland display
    application.wl_disp = wl_display_connect(NULL);
    LV_ASSERT_MSG(application.wl_disp, "failed to connect to Wayland server");
    if ((!application.wl_disp) || (!application.xkb_context)) {
        return false;
    }

    /* Add registry listener and wait for registry reception */
    application.format = WL_SHM_FORMAT_ARGB8888;
    application.registry = wl_display_get_registry(application.wl_disp);
    wl_registry_add_listener(application.registry, &registry_listener, &application);
    wl_display_dispatch(application.wl_disp);
    wl_display_roundtrip(application.wl_disp);

    LV_ASSERT_MSG(application.compositor, "Wayland compositor not available");
    LV_ASSERT_MSG(application.shm, "Wayland SHM not available");
    LV_ASSERT_MSG((application.format != 0xFFFFFFFF), "WL_SHM_FORMAT not available");
    if ((application.compositor == NULL) || (application.shm == NULL) ||
        (application.format == 0xFFFFFFFF)) {
        return false;
    }

    smm_init(&evs, &application);

#if LV_WAYLAND_CLIENT_SIDE_DECORATIONS
    const char* env_disable_decorations = getenv("LV_WAYLAND_DISABLE_WINDOWDECORATION");
    application.opt_disable_decorations = ((env_disable_decorations != NULL) &&
        (env_disable_decorations[0] != '0'));
#endif

    _lv_ll_init(&application.window_ll, sizeof(lw_window_t));

    application.cycle_timer = lv_timer_create(handle_io_lv_cb, LW_CYCLE_PERIOD, NULL);
    LV_ASSERT_MSG(application.cycle_timer, "failed to create cycle timer");
    if (!application.cycle_timer) {
        return false;
    }

    return true;
}

/**
 * De-initialize Wayland driver
 */
void lv_wayland_deinit(void)
{
    lw_window_t* window = NULL;

    _LV_LL_READ(&application.window_ll, window)
    {
        if (!window->closed) {
            lv_display_delete(window->display);
            destroy_wl_window(window);
        }
    }

    smm_deinit();
    if (application.shm) {
        wl_shm_destroy(application.shm);
    }

#if LV_WAYLAND_XDG_SHELL
    if (application.xdg_wm) {
        xdg_wm_base_destroy(application.xdg_wm);
    }
#endif

    if (application.wl_seat) {
        wl_seat_destroy(application.wl_seat);
    }

    if (application.subcompositor) {
        wl_subcompositor_destroy(application.subcompositor);
    }

    if (application.compositor) {
        wl_compositor_destroy(application.compositor);
    }

    wl_registry_destroy(application.registry);
    wl_display_flush(application.wl_disp);
    wl_display_disconnect(application.wl_disp);

    _lv_ll_clear(&application.window_ll);
}

#endif // LV_USE_WAYLAND
