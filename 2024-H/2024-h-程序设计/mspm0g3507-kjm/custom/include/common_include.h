// common.h
#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

#include "FreeRTOSConfig.h"

#include <stdio.h>
#include "ti_msp_dl_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "oled_driver.h"
#include "gray_detection_app.h"
#include "motor_hardware.h"
#include "encoder.h"
#include "timers.h"
#include "motor_app.h"
#include "embedfire_protocol.h"
#include "serialplot_protocol.h"
#include "button_app.h"
#include "menu_app.h"
#include "encoder_app.h"
#include "tests.h"
#include "beep.h"
#include "rgb_led.h"
#include "alert.h"
#include "periodic_event_task.h"
#include "menu_logic.h"
#include "car_controller.h"
#include "car_state_machine.h"
#include "wit_jyxx.h"

#endif // COMMON_H