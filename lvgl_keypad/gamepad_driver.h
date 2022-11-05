#ifndef LV_GAMEPAD_DRIVER_H
#define LV_GAMEPAD_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "lvgl.h"


#define A       19
#define B       27
#define START   32
#define SELECT  33
#define IO_Y    ADC1_CHANNEL_7
#define IO_X    ADC1_CHANNEL_0
#define MENU    25

enum gampad_input_e
{
	GAMEPAD_INPUT_UP = 0x01,
    GAMEPAD_INPUT_DOWN = 0x02,
    GAMEPAD_INPUT_LEFT = 0x04,
    GAMEPAD_INPUT_RIGHT = 0x08,
    GAMEPAD_INPUT_A = 0x10,
    GAMEPAD_INPUT_B = 0x20,
    GAMEPAD_INPUT_SELECT = 0x40,
    GAMEPAD_INPUT_START = 0x80
};


void gamepad_driver_init();

uint8_t gamepad_driver_read(void);
void lv_gamepad_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#endif

