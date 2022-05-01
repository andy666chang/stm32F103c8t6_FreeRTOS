#ifndef _BOARD_TASK_H_
#define _BOARD_TASK_H_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define mainTIME_TASK_PRIORITY ( tskIDLE_PRIORITY + 4 )

// APP task
void led_toggle_task(void *pvParameters);

// Init task
void board_init_task_create(void);

#endif _BOARD_TASK_H_
