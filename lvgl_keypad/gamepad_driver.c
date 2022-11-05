/**
 * @file gamepad_driver.c
 */
#include "sdkconfig.h"
#include "gamepad_driver.h"
#if defined (CONFIG_LV_GAMEPAD_M5STACK_FACE)
#include "m5stack_face_gamepad.h"
#endif

void gamepad_driver_init(void)
{
#if defined (CONFIG_LV_GAMEPAD_M5STACK_FACE)
    m5stack_gamepad_driver_init();
#endif
}

uint8_t gamepad_driver_read(void)
{
#if defined (CONFIG_LV_GAMEPAD_M5STACK_FACE)
    return m5stack_gamepad_driver_read();
#endif
}

static uint8_t lv_gamepad_driver_process_key(uint8_t key_value, lv_indev_data_t *data)
{
    uint8_t processed_keys; 
    if(key_value & GAMEPAD_INPUT_UP)
    {
        //printf("UP\n");
        data->key = LV_KEY_UP;
        processed_keys = GAMEPAD_INPUT_UP;
    }
    else if(key_value & GAMEPAD_INPUT_DOWN)
    {
        //printf("DOWN\n");
        data->key = LV_KEY_DOWN;
        processed_keys = GAMEPAD_INPUT_DOWN;
    }
    else if(key_value & GAMEPAD_INPUT_LEFT)
    {
        //printf("LEFT\n");
        data->key = LV_KEY_LEFT;
        processed_keys = GAMEPAD_INPUT_LEFT;
    }
    else if(key_value & GAMEPAD_INPUT_RIGHT)
    {
        //printf("RIGHT\n");
        data->key = LV_KEY_RIGHT;
        processed_keys = GAMEPAD_INPUT_RIGHT;
    }
    else if(key_value & GAMEPAD_INPUT_B)
    {
        //printf("ESC\n");
        data->key = LV_KEY_ESC;
        processed_keys = GAMEPAD_INPUT_B;
    }
    else if(key_value & GAMEPAD_INPUT_A)
    {
        //printf("ENTER\n");
        data->key = LV_KEY_ENTER;
        processed_keys = GAMEPAD_INPUT_A;
    }
    else if(key_value & GAMEPAD_INPUT_SELECT)
    {
        //printf("SELECT\n");
        data->key = LV_KEY_NEXT;
        processed_keys = GAMEPAD_INPUT_SELECT;
    }
    else if(key_value & GAMEPAD_INPUT_START)
    {
        //printf("START\n");
        data->key = LV_KEY_PREV;
        processed_keys = GAMEPAD_INPUT_START;
    }
    else
    {
        return 0;
    }
    return processed_keys;
}

void lv_gamepad_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint8_t key_value;
    static uint8_t processed_keys = 0;
    uint8_t processed_key;
    
    data->continue_reading = false;
    
    key_value = gamepad_driver_read();
    if(key_value == 0xFF)
    {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    processed_key = lv_gamepad_driver_process_key(key_value & (~processed_keys), data);
    
    if(processed_key)
    {
        data->state = LV_INDEV_STATE_PR;
        processed_keys |= processed_key;
        if(key_value & (~processed_keys))
            data->continue_reading = true;
        return ;
    }
    processed_key = lv_gamepad_driver_process_key((~key_value) & processed_keys, data);
    if(processed_key)
    {
        data->state = LV_INDEV_STATE_REL;
        processed_keys &= ~processed_key;
        if((~key_value) & processed_keys)
            data->continue_reading = true;
        return;
    }
    if(key_value != 0)
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;
    data->continue_reading = false;
}

