#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#if LV_USE_DEMO_MULTILANG

#ifndef LV_ATTRIBUTE_MEM_ALIGN
    #define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMAGE_IMG_EMOJI_ROCKET
    #define LV_ATTRIBUTE_IMAGE_IMG_EMOJI_ROCKET
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMAGE_IMG_EMOJI_ROCKET uint8_t img_emoji_rocket_map[]
= {
    0xff, 0xf8, 0xf4, 0xff, 0xff, 0xfe, 0xfb, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xf8, 0xfa, 0xff, 0xff, 0xff, 0xff, 0xec, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xf4, 0xff, 0xff, 0xff, 0xff, 0xeb, 0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xf5, 0xff, 0xff, 0xfd, 0xff, 0xff,
    0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xfb, 0xfb, 0xfb, 0xff, 0xf2, 0xf7, 0xf6, 0xff, 0xf5, 0xfa, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xea, 0xf0, 0xeb, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xfd, 0xf6, 0xff, 0xff, 0xee, 0xfb, 0xfd, 0xff, 0xef, 0xf4, 0xff, 0xff, 0xea, 0xe1, 0xff, 0xff, 0xfd, 0xff, 0xfe, 0xff,
    0xff, 0xfd, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xf2, 0xee, 0xff, 0xff, 0xa4, 0xa8, 0xe9, 0xff, 0x79, 0x87, 0xd9, 0xff, 0x5b, 0x5d, 0xe1, 0xff, 0x55, 0x4e, 0xc7, 0xff, 0xaf, 0xba, 0xd6, 0xff,
    0xf4, 0xf4, 0xfa, 0xff, 0xfd, 0xfe, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xf9, 0xfe, 0xfd, 0xff, 0xf9, 0xfb, 0xfc, 0xff, 0xfc, 0xfd, 0xff, 0xff, 0xfd, 0xfd, 0xff, 0xff, 0xff, 0xf4, 0xff, 0xff, 0xe5, 0xf1, 0xcd, 0xff, 0xc4, 0xd4, 0xda, 0xff, 0x46, 0x4a, 0xc0, 0xff, 0x42, 0x4a, 0xf1, 0xff, 0x2e, 0x35, 0xe2, 0xff, 0x39, 0x3a, 0xca, 0xff, 0xa0, 0xa7, 0xf6, 0xff,
    0xfd, 0xfd, 0xff, 0xff, 0xfd, 0xfe, 0xff, 0xff, 0xfd, 0xfe, 0xfc, 0xff, 0xf9, 0xfa, 0xf6, 0xff, 0xfa, 0xfb, 0xf7, 0xff, 0xfe, 0xff, 0xfd, 0xff, 0xfb, 0xfc, 0xff, 0xff, 0xf6, 0xf6, 0xfc, 0xff, 0xd8, 0xc3, 0xe4, 0xff, 0xe2, 0xee, 0xc4, 0xff, 0xde, 0xef, 0xec, 0xff, 0x6b, 0x6f, 0xdf, 0xff, 0x36, 0x3d, 0xe8, 0xff, 0x30, 0x34, 0xea, 0xff, 0x3f, 0x41, 0xd5, 0xff, 0xb5, 0xbf, 0xff, 0xff,
    0xfd, 0xfd, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xfb, 0xf6, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xf1, 0xf1, 0xf1, 0xff, 0xd6, 0xd6, 0xdc, 0xff, 0xe5, 0xdb, 0xe1, 0xff, 0xd8, 0xd8, 0xc6, 0xff, 0xdb, 0xdd, 0xe7, 0xff, 0xd4, 0xdc, 0xff, 0xff, 0x6b, 0x77, 0xd5, 0xff, 0x47, 0x43, 0xdf, 0xff, 0x4a, 0x43, 0xca, 0xff, 0xe4, 0xf5, 0xff, 0xff,
    0xff, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf4, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xf5, 0xf0, 0xe7, 0xff, 0xda, 0xd6, 0xd5, 0xff, 0xc3, 0xc0, 0xc2, 0xff, 0xda, 0xd4, 0xd5, 0xff, 0x60, 0x5d, 0x48, 0xff, 0x57, 0x4f, 0x48, 0xff, 0xbe, 0xbb, 0xc4, 0xff, 0xb4, 0xbc, 0xd3, 0xff, 0x92, 0x8f, 0xde, 0xff, 0x97, 0x8e, 0xde, 0xff, 0xf6, 0xff, 0xf9, 0xff,
    0xfd, 0xfb, 0xfb, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xf1, 0xe7, 0xd5, 0xff, 0xd2, 0xc8, 0xb6, 0xff, 0xc3, 0xbb, 0xae, 0xff, 0xc3, 0xbf, 0xba, 0xff, 0xcb, 0xc9, 0xc9, 0xff, 0xd1, 0xc6, 0xe0, 0xff, 0x52, 0x57, 0x24, 0xff, 0x6c, 0x65, 0x3a, 0xff, 0xb2, 0x9e, 0xa3, 0xff, 0xe0, 0xdd, 0xd9, 0xff, 0xbd, 0xc3, 0xbe, 0xff, 0xea, 0xeb, 0xf5, 0xff, 0xfc, 0xfe, 0xf8, 0xff,
    0xef, 0xf3, 0xff, 0xff, 0xff, 0xf6, 0xe9, 0xff, 0xda, 0xb0, 0x57, 0xff, 0xcc, 0x8a, 0x00, 0xff, 0xc0, 0x91, 0x1e, 0xff, 0xac, 0xa4, 0x7f, 0xff, 0xc4, 0xc1, 0xca, 0xff, 0xd3, 0xbb, 0xc3, 0xff, 0xd9, 0xd1, 0xd2, 0xff, 0xbb, 0xb0, 0xa8, 0xff, 0xba, 0xad, 0x9f, 0xff, 0xd7, 0xce, 0xc4, 0xff, 0xca, 0xcc, 0xcd, 0xff, 0xc6, 0xcd, 0xd6, 0xff, 0xea, 0xf0, 0xf7, 0xff, 0xfd, 0xfe, 0xff, 0xff,
    0xf8, 0xff, 0xfb, 0xff, 0xe0, 0xde, 0xcc, 0xff, 0xce, 0xab, 0x79, 0xff, 0xca, 0x8c, 0x3e, 0xff, 0xa7, 0x7a, 0x41, 0xff, 0xc3, 0xbd, 0xb6, 0xff, 0x9c, 0xa4, 0xa4, 0xff, 0x5b, 0x50, 0x3a, 0xff, 0x97, 0x9d, 0xa8, 0xff, 0xc2, 0xc5, 0xc9, 0xff, 0xb4, 0xb4, 0xb4, 0xff, 0xb9, 0xbb, 0xbb, 0xff, 0xc5, 0xca, 0xcd, 0xff, 0xef, 0xf5, 0xfa, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfa, 0xf7, 0xf2, 0xff,
    0xff, 0xff, 0xef, 0xff, 0xf8, 0xfa, 0xf4, 0xff, 0xff, 0xff, 0xf2, 0xff, 0xff, 0xf5, 0xd2, 0xff, 0xc4, 0xa5, 0x8e, 0xff, 0xbd, 0xbf, 0xc7, 0xff, 0x87, 0x8f, 0x96, 0xff, 0x75, 0x6f, 0x58, 0xff, 0x97, 0xa1, 0xa8, 0xff, 0xb8, 0xc2, 0xc9, 0xff, 0xaf, 0xb9, 0xc0, 0xff, 0xc7, 0xd1, 0xd8, 0xff, 0xe1, 0xea, 0xee, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xf4, 0xff,
    0xff, 0xf7, 0xfa, 0xff, 0xf8, 0xfa, 0xff, 0xff, 0xf4, 0xfd, 0xff, 0xff, 0xd8, 0xd5, 0xb6, 0xff, 0xbf, 0xb8, 0x97, 0xff, 0xc1, 0xc7, 0xc6, 0xff, 0xb0, 0xaf, 0xbf, 0xff, 0xd6, 0xc0, 0xc5, 0xff, 0xce, 0xc4, 0xa6, 0xff, 0xab, 0xa7, 0x95, 0xff, 0xd1, 0xd6, 0xd4, 0xff, 0xec, 0xf5, 0xf9, 0xff, 0xfa, 0xff, 0xff, 0xff, 0xf6, 0xf7, 0xf3, 0xff, 0xfa, 0xfb, 0xf2, 0xff, 0xff, 0xff, 0xf9, 0xff,
    0xff, 0xf7, 0xee, 0xff, 0xe6, 0xfa, 0xff, 0xff, 0xa8, 0xcb, 0xec, 0xff, 0x61, 0x73, 0x72, 0xff, 0x81, 0x82, 0x6e, 0xff, 0xa6, 0xa5, 0xa1, 0xff, 0xca, 0xc3, 0xc8, 0xff, 0xb9, 0x9e, 0x9a, 0xff, 0xd8, 0xab, 0x5a, 0xff, 0xa0, 0x81, 0x4c, 0xff, 0xff, 0xff, 0xea, 0xff, 0xf7, 0xf6, 0xf2, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xfd, 0xff, 0xfe, 0xff, 0xf2, 0xfe, 0xff, 0xff,
    0xfb, 0xff, 0xe4, 0xff, 0xac, 0xe3, 0xfe, 0xff, 0x63, 0xa4, 0xef, 0xff, 0x3d, 0x5a, 0x97, 0xff, 0x5e, 0x5b, 0x74, 0xff, 0x86, 0x7f, 0x84, 0xff, 0xe0, 0xdb, 0xcc, 0xff, 0xbb, 0xaf, 0x85, 0xff, 0xc5, 0x80, 0x11, 0xff, 0x96, 0x6b, 0x22, 0xff, 0xff, 0xff, 0xe9, 0xff, 0xf5, 0xf1, 0xf0, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xea, 0xf8, 0xff, 0xff,
    0xd6, 0xff, 0xf2, 0xff, 0x49, 0xa5, 0xd8, 0xff, 0x00, 0x61, 0xcb, 0xff, 0x1f, 0x58, 0xad, 0xff, 0x7f, 0x87, 0xac, 0xff, 0xc0, 0xbc, 0xc2, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xe4, 0xff, 0xc8, 0x87, 0x1a, 0xff, 0xc1, 0x9b, 0x5b, 0xff, 0xff, 0xf8, 0xeb, 0xff, 0xf7, 0xf8, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xf8, 0xf6, 0xff, 0xfd, 0xfb, 0xfb, 0xff, 0xf3, 0xfd, 0xff, 0xff,
    0xa1, 0xe7, 0xfe, 0xff, 0x38, 0xac, 0xff, 0xff, 0x0c, 0x91, 0xff, 0xff, 0x49, 0xa0, 0xe0, 0xff, 0xc7, 0xe6, 0xef, 0xff, 0xfa, 0xff, 0xfe, 0xff, 0xf0, 0xf2, 0xfc, 0xff, 0xfb, 0xfb, 0xff, 0xff, 0xda, 0xa4, 0x46, 0xff, 0xff, 0xf2, 0xc3, 0xff, 0xfa, 0xfe, 0xff, 0xff, 0xef, 0xf7, 0xff, 0xff, 0xfd, 0xfa, 0xff, 0xff, 0xff, 0xfd, 0xfb, 0xff, 0xff, 0xfb, 0xfb, 0xff, 0xf8, 0xfe, 0xff, 0xff,
    0x93, 0xea, 0xf4, 0xff, 0x97, 0xdf, 0xe9, 0xff, 0xad, 0xe0, 0xea, 0xff, 0xd9, 0xf3, 0xfa, 0xff, 0xf8, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfd, 0xfa, 0xff, 0xff, 0xfe, 0xfa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xd4, 0xff, 0xff, 0xff, 0xd6, 0xf9, 0xff, 0xff, 0xe1, 0xf6, 0xfe, 0xff, 0xf6, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xfa, 0xf9, 0xff, 0xfd, 0xfc, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfd, 0xfd, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xff, 0xf8, 0xfd, 0xfc, 0xff, 0xf6, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

const lv_image_dsc_t img_emoji_rocket = {
    .header.cf = LV_COLOR_FORMAT_ARGB8888,
    .header.w = 16,
    .header.h = 19,
    .data = img_emoji_rocket_map,
};

#endif
