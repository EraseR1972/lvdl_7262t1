#if LV_BUILD_TEST
#include "../lvgl.h"

#include "unity/unity.h"

void test_arc_creation_successfull(void);
void test_arc_should_truncate_to_max_range_when_new_value_exceeds_it(void);
void test_arc_should_truncate_to_min_range_when_new_value_is_inferior(void);
void test_arc_should_update_value_after_updating_range(void);
void test_arc_angles_when_reversed(void);

static lv_obj_t *active_screen = NULL;
static lv_obj_t *arc = NULL;

void test_arc_creation_successfull(void)
{
    active_screen = lv_scr_act();
    arc = lv_arc_create(active_screen);

    TEST_ASSERT_NOT_NULL(arc);
}

void test_arc_should_truncate_to_max_range_when_new_value_exceeds_it(void)
{
    /* Default max range is 100 */
    int16_t value_after_truncation = 100;

    active_screen = lv_scr_act();
    arc = lv_arc_create(active_screen);

    lv_arc_set_value(arc, 200);

    TEST_ASSERT_EQUAL_INT16(value_after_truncation, lv_arc_get_value(arc));
}

void test_arc_should_truncate_to_min_range_when_new_value_is_inferior(void)
{
    /* Default min range is 100 */
    int16_t value_after_truncation = 1;

    active_screen = lv_scr_act();
    arc = lv_arc_create(active_screen);

    lv_arc_set_value(arc, 0);

    TEST_ASSERT_EQUAL_INT16(value_after_truncation, lv_arc_get_value(arc));
}

void test_arc_should_update_value_after_updating_range(void)
{
    int16_t value_after_updating_max_range = 50;
    int16_t value_after_updating_min_range = 30;

    active_screen = lv_scr_act();
    arc = lv_arc_create(active_screen);

    lv_arc_set_value(arc, 80);
    lv_arc_set_range(arc, 1, 50);

    TEST_ASSERT_EQUAL_INT16(value_after_updating_max_range, lv_arc_get_value(arc));

    lv_arc_set_value(arc, 10);
    lv_arc_set_range(arc, 30, 50);

    TEST_ASSERT_EQUAL_INT16(value_after_updating_min_range, lv_arc_get_value(arc));
}

/* See #2522 for more information */
void test_arc_angles_when_reversed(void)
{
    uint16_t expected_start_angle = 36;
    uint16_t expected_end_angle = 90;
    int16_t expected_value = 40;

    lv_obj_t *arcBlack;
    arcBlack = lv_arc_create(lv_scr_act());

    lv_arc_set_mode(arcBlack, LV_ARC_MODE_REVERSE);

    lv_arc_set_bg_angles(arcBlack, 0, 90);
    
    lv_arc_set_value(arcBlack, expected_value);

    TEST_ASSERT_EQUAL_UINT16(expected_start_angle, lv_arc_get_angle_start(arcBlack));
    TEST_ASSERT_EQUAL_UINT16(expected_end_angle, lv_arc_get_angle_end(arcBlack));
    TEST_ASSERT_EQUAL_INT16(expected_value, lv_arc_get_value(arcBlack));
}

#endif
