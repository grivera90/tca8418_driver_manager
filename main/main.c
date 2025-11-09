/******************************************************************************
    Includes
******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>

#include "bsp.h"
#include "freertos/projdefs.h"
#include "keyboard_mgr.h"
#include "portmacro.h"
#include "tca9554_drv.h"
/******************************************************************************
    Defines and constants
******************************************************************************/
#define READ_KEYBOARD_TASK_PRIORITY				(tskIDLE_PRIORITY + 16)
#define READ_IO_EXPANDER_TASK_PRIORITY			(tskIDLE_PRIORITY + 3)
#define KEY_MATCH(_key, _code, _event)			((_key)->press_code == (_code) && (_key)->event == (_event))
/******************************************************************************
    Data types
******************************************************************************/

/******************************************************************************
    Local variables
******************************************************************************/
static QueueHandle_t keyboard_event_queue = NULL; 
static kb_mgr_lenguage_t keyboard_lenguage = LENGUAGE_SPANISH;
static const char *MODULE_NAME = "[MAIN_APP]";
/******************************************************************************
    Local function prototypes
******************************************************************************/
static void read_keyboard_task(void *pvParameters);
static void process_on_key(kb_mgr_key_prop_t *key_prop);
static void read_io_expander_task(void *pvParamaters);
/******************************************************************************
    Main loop
******************************************************************************/
void app_main(void)
{
	
	bsp_init();
	bsp_gpio_expander_init();
	
	bsp_led_ok_set(true);
	bsp_led_error_set(true);
	bsp_led_alarm_set(true);

	vTaskDelay(pdMS_TO_TICKS(1000));

	bsp_led_ok_set(false);
	bsp_led_error_set(false);
	bsp_led_alarm_set(false);
	
	vTaskDelay(pdMS_TO_TICKS(100));
	
	bsp_keypad_scan_device_init();
	
	/* Keyboard Configs. */
	keyboard_event_queue = xQueueCreate(1, sizeof(kb_mgr_key_prop_t));
	keyboard_mgr_init(keyboard_event_queue, keyboard_lenguage);
	
    xTaskCreate(read_keyboard_task, "read_keyboard_task", 
                                        configMINIMAL_STACK_SIZE * 6, 
                                        NULL, 
                                        READ_KEYBOARD_TASK_PRIORITY, 
                                        NULL);
                                        
    xTaskCreate(read_io_expander_task, "read_io_expander_task", 
                                        configMINIMAL_STACK_SIZE * 2, 
                                        NULL, 
                                        READ_IO_EXPANDER_TASK_PRIORITY, 
                                        NULL);                                        
    while (true) 
    {
        printf("Hello from app_main!\n");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
/******************************************************************************
    Local function definitions
******************************************************************************/
static void read_keyboard_task(void *pvParameters)
{
	kb_mgr_key_prop_t key_prop = {0};
	
	printf("%s read keyboard task is running\n", MODULE_NAME);
	
	while(1)
	{	
		if (pdTRUE == xQueueReceive(keyboard_event_queue, &key_prop, portMAX_DELAY))
		{
			/* Process On Key */
			process_on_key(&key_prop);
		}
	}
}

static void process_on_key(kb_mgr_key_prop_t *key_prop)
{
	
	if (KEY_PRESS == key_prop->event || KEY_RELEASE == key_prop->event)
	{
		printf("%s event: %d, ascii code: %c, %s\n", MODULE_NAME, 
													key_prop->event, 
													((key_prop->ascii_value <= 125 && key_prop->ascii_value >= 32) ? key_prop->ascii_value : ' '), 
													((key_prop->event & 0x80) ? "key press" : "key released"));
	}
	else
	{
		/* wrong event */
		printf("%s wrong event\n", MODULE_NAME);
	}	
}

static void read_io_expander_task(void *pvParamaters)
{
	printf("%s read io expander task is running\n", MODULE_NAME);
	
	while(1)
	{
		
		bool sw1 = bsp_read_sw_1();
		bool sw2 = bsp_read_sw_2();
		bool sw3 = bsp_read_sw_3();
		
		printf("%s sw1: %s, sw1: %s, sw1: %s\n", MODULE_NAME, 
												(sw1 == true) ? "true":"false",
												(sw2 == true) ? "true":"false",
												(sw3 == true) ? "true":"false");
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}









