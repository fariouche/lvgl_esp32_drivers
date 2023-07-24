#include <stdio.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "sdkconfig.h"
#include "gamepad_driver.h"
#include "lvgl_i2c/i2c_manager.h"

#if defined(CONFIG_LV_PREDEFINED_DISPLAY_M5CORE2)
#define GPIO_GAMEPAD_INT 33
#else
#define GPIO_GAMEPAD_INT 5
#endif

#define GAMEPAD_I2C_ADDR 0x08

#define M5GB_KEY_UP         0x01
#define M5GB_KEY_DOWN       0x02
#define M5GB_KEY_LEFT       0x04
#define M5GB_KEY_RIGHT      0x08
#define M5GB_KEY_A          0x10
#define M5GB_KEY_B          0x20
#define M5GB_KEY_SELECT     0x40
#define M5GB_KEY_START      0x80

static bool input_task_is_running = false;
static uint8_t keys = 0xFF;
static bool input_gamepad_initialized = false;
static SemaphoreHandle_t xSemaphore;

static uint8_t m5stack_gamepad_driver_read_internal(void)
{
    //if (!input_gamepad_initialized) 
    //    return 0xFF;
    
    static uint8_t prev_retval = 0;
	uint8_t retval = 0;
	uint8_t key_value = 0xff;
	esp_err_t err;

	if(gpio_get_level(GPIO_GAMEPAD_INT) == 0) 
	{
	    err = i2c_manager_read(I2C_NUM_0, GAMEPAD_I2C_ADDR, I2C_NO_REG, &key_value, 1);
	    if(err != ESP_OK)
	        return 0xFF;

        printf("key is:0x%x \r\n", key_value);

		if((key_value & M5GB_KEY_A) == 0)
			retval |=  GAMEPAD_INPUT_A;
		else 
			retval &= ~(GAMEPAD_INPUT_A);

		if((key_value & M5GB_KEY_B) == 0)
			retval |=  GAMEPAD_INPUT_B;
		else
			retval &= ~(GAMEPAD_INPUT_B);

		if((key_value & M5GB_KEY_UP) == 0)
			retval |=  GAMEPAD_INPUT_UP;
		else
			retval &= ~(GAMEPAD_INPUT_UP);

		if((key_value & M5GB_KEY_DOWN) == 0)
			retval |=  GAMEPAD_INPUT_DOWN;
		else
			retval &= ~(GAMEPAD_INPUT_DOWN);

		if((key_value & M5GB_KEY_LEFT) == 0)
			retval |=  GAMEPAD_INPUT_LEFT;
		else
			retval &= ~(GAMEPAD_INPUT_LEFT);

		if((key_value & M5GB_KEY_RIGHT) == 0)
			retval |=  GAMEPAD_INPUT_RIGHT;
		else
			retval &= ~(GAMEPAD_INPUT_RIGHT);

		if((key_value & M5GB_KEY_SELECT) == 0)
			retval |=  GAMEPAD_INPUT_SELECT;
		else
			retval &= ~(GAMEPAD_INPUT_SELECT);

		if((key_value & M5GB_KEY_START) == 0)
			retval |=  GAMEPAD_INPUT_START;
		else
			retval &= ~(GAMEPAD_INPUT_START);
	
	    prev_retval = retval;
	}

	return prev_retval;
}



static void input_task(void *arg)
{
    input_task_is_running = true;
    printf("m5stack gamepad input task started\n");
    while(input_task_is_running)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        
        keys = m5stack_gamepad_driver_read_internal();
    }
}

static void IRAM_ATTR m5stack_handle_gamepad_interrupt(void * arg) 
{
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}

void m5stack_gamepad_driver_init(void)
{
    esp_err_t err;
    
    xSemaphore = xSemaphoreCreateCounting(10, 0);
    //xSemaphore = xSemaphoreCreateBinary();
    if(xSemaphore == NULL)
    {
        printf("xSemaphoreCreateMutex failed.\n");
        abort();
    }


    if(i2c_manager_write(I2C_NUM_0, GAMEPAD_I2C_ADDR, I2C_NO_REG, NULL, 0) != ESP_OK)
    {
        printf("**** gamepad not found, ignoring\n");
        return;
    }
    
    gpio_set_direction(GPIO_GAMEPAD_INT, GPIO_MODE_INPUT);
	gpio_pullup_en(GPIO_GAMEPAD_INT);
	m5stack_gamepad_driver_read_internal();//flush any pending read
	err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	if(err != ESP_OK)
	{
	    printf("m5stack gamepad driver failed: gpio_install_isr_service: %d\n", err);
	    return;
	}
	gpio_set_intr_type(GPIO_GAMEPAD_INT, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_GAMEPAD_INT, m5stack_handle_gamepad_interrupt, NULL);

    // Start background polling
    xTaskCreatePinnedToCore(input_task, "input_task", 1024 * 2, NULL, 5, NULL, 1);
    
    gpio_intr_enable(GPIO_GAMEPAD_INT);
    
    input_gamepad_initialized = true;
    
    printf("m5stack_gamepad_driver_init done.\n");
}

void m5stack_gamepad_driver_terminate(void)
{
    printf("***** m5stack gamepad terminate not IMPLEMENTED\n");
    input_gamepad_initialized = false;
}

uint8_t m5stack_gamepad_driver_read(void)
{
    return keys;
}


