#! ../run_testcase.py
#LVGL MicroPython binding tester script: Widget-creation/deletion and basic flag/property-settings


#test-objects:

lv_testbutton = lv.button( lv.scr_act() )


#tests:

lv_testbutton.add_flag( lv.obj.FLAG.CHECKABLE )
lv_test.assert_equal( lv_testbutton.has_flag( lv.obj.FLAG.CHECKABLE ), True, "Widget Flag-setting Test: CHECKABLE" )

lv_testbutton.add_flag( lv.obj.FLAG.FLOATING )
lv_test.assert_equal( lv_testbutton.has_flag( lv.obj.FLAG.FLOATING ), True, "Widget Flag-setting Test: FLOATING" )

