#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "touch.h" // 引入电容触摸头文件

static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[screenWidth*30];

TFT_eSPI my_lcd = TFT_eSPI();

static lv_obj_t * txtfield = nullptr;
static const char * btnm_map[] = {
	"1", "2", "3", "4", "5", "\n",
	"6", "7", "8", "9", "0", "\n",
	"Action1", "Action2", NULL
};

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);
	my_lcd.setAddrWindow(area->x1, area->y1,  w, h);
	my_lcd.pushColors((uint16_t *)&color_p->full, w*h, true);
	lv_disp_flush_ready(disp);
}

/* Reading input device (capacitive touch here) */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
	if (touch_touched()) {
		data->state = LV_INDEV_STATE_PR;
		data->point.x = touch_last_x;
		data->point.y = touch_last_y;
	} else {
		data->state = LV_INDEV_STATE_REL;
	}
}

static void event_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	if(code == LV_EVENT_VALUE_CHANGED) {
		uint32_t id = lv_btnmatrix_get_selected_btn(obj);
		const char * txt = lv_btnmatrix_get_btn_text(obj, id);

		char buff[50];
		sprintf(buff, "%s was pressed\n", txt);
		lv_textarea_set_cursor_pos(txtfield, LV_TEXTAREA_CURSOR_LAST );
		lv_textarea_add_text(txtfield, buff);
	}
}


void show_mem() {
	char buff[50];
	sprintf(buff, "=== RAM ===\n");
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Heap total: %u bytes\n", ESP.getHeapSize());
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Heap free: %u bytes\n", ESP.getFreeHeap());
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Max alloc: %u bytes\n", ESP.getMaxAllocHeap());
	lv_textarea_add_text(txtfield, buff);

	sprintf(buff, "\n=== PSRAM ===\n");
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "PSRAM total: %u bytes\n", ESP.getPsramSize());
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "PSRAM free: %u bytes\n", ESP.getFreePsram());
	lv_textarea_add_text(txtfield, buff);

	sprintf(buff, "\n=== FLASH ===\n");
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Flash size: %u bytes\n", ESP.getFlashChipSize());
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Flash speed: %u Hz\n", ESP.getFlashChipSpeed());
	lv_textarea_add_text(txtfield, buff);
	sprintf(buff, "Flash mode: %u\n\n\n", ESP.getFlashChipMode());
	lv_textarea_add_text(txtfield, buff);
}

void lv_example_btnmatrix_1(void)
{
	lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
	lv_obj_set_style_width(btnm1, lv_pct(95), LV_PART_MAIN );
	lv_btnmatrix_set_map(btnm1, btnm_map);
	//lv_btnmatrix_set_btn_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
	lv_btnmatrix_set_btn_ctrl(btnm1, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
	lv_btnmatrix_set_btn_ctrl(btnm1, 11, LV_BTNMATRIX_CTRL_CHECKED);
	lv_obj_align(btnm1, LV_ALIGN_BOTTOM_MID, 0, -10);
	lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);

	txtfield = lv_textarea_create(lv_scr_act());
	lv_obj_set_style_width(txtfield, lv_pct(95), LV_PART_MAIN );
	lv_obj_align(txtfield, LV_ALIGN_TOP_MID, 0, 10);


}


void setup(){
    Serial.begin(115200); /* prepare for possible serial debug */
    while (!Serial);
    String LVGL_Arduino = "Hello ESP32-S3! ";
    LVGL_Arduino += String("LVGL V") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    my_lcd.init();
    my_lcd.setRotation(2);
    my_lcd.invertDisplay(1);

    touch_init(my_lcd.width(), my_lcd.height(), my_lcd.getRotation()); // 初始化电容触摸

    lv_init();
    delay(10);

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, screenWidth*30);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = my_lcd.width();
    disp_drv.ver_res = my_lcd.height();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    lv_example_btnmatrix_1();
	lv_obj_t * label = lv_label_create(lv_scr_act());
	lv_obj_center(label);
	lv_label_set_text(label, LVGL_Arduino.c_str());
	show_mem();

}


void loop(){
    lv_task_handler(); /* let the GUI do its work */
    delay(5);
}