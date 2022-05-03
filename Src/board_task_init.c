
#include "board_task.h"
#include "SEGGER_RTT.h"

// APP task
void led_toggle_task(void *pvParameters) {
    while (1)
    {
        HAL_GPIO_TogglePin(DEBUG_LEB_GPIO_Port, DEBUG_LEB_Pin);
        vTaskDelay( 500 );
    }
    vTaskDelete(NULL);
}


// Init task
void board_init_task(void *pvParameters) {
    // init function
    xTaskCreate( led_toggle_task, (const char *) "LED_toggle", configMINIMAL_STACK_SIZE,NULL, mainTIME_TASK_PRIORITY, NULL );

    extern void cli_task(void *pvParameters);
    xTaskCreate( cli_task, (const char *) "cli_task" , 512,NULL, mainTIME_TASK_PRIORITY, NULL );

    vTaskDelete(NULL);
}

void board_init_task_create(void) {
    // Create init task function
    xTaskCreate( board_init_task, (const char *) "board_init_task", configMINIMAL_STACK_SIZE,NULL, mainTIME_TASK_PRIORITY, NULL );
    SEGGER_RTT_printf(0, "RTOS Start!\n");
}
