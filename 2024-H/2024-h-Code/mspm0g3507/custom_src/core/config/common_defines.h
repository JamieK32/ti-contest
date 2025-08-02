#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#include "stdint.h"

// 编译时信息
#define UNIT_TEST_MODE 1
#define FIRMWARE_VERSION "v1.0.0"

// 编译时检查
#if (UNIT_TEST_MODE == 1)

#endif

#define TASK_TYPE_21F 	 1
#define TASK_TYPE_22C    2
#define TASK_TYPE_24H    3


// 当前选择的任务
#define CURRENT_TASK_TYPE TASK_TYPE_24H

// 根据任务类型包含对应配置
#if (CURRENT_TASK_TYPE == TASK_TYPE_24H)
    #include "task24h_config.h"
#elif (CURRENT_TASK_TYPE == TASK_TYPE_22C)
    #include "task22c_config.h"
#elif (CURRENT_TASK_TYPE == TASK_TYPE_21F)
		#include "task21f_config.h"
#else
    #error "Please select a valid task type!"
#endif



#endif // COMMON_DEFINES_H