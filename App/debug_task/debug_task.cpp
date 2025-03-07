/**
 * @file debug.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "debug_task.h"
#include "topics.h"
#include "bsp_bitband.h"
#include "bsp_usart.h"
#include "vofa.h"
#include "air_joy.h"
#include "arm_math.h"

#ifdef CHASSIS_TO_DEBUG
#include "pid_controller.h"
#endif 
// extern Motor_C610 m2006;

osThreadId_t Debug_TaskHandle;

///////////////////////////////////////////////////

uint16_t left_y;

Uart_Tx_Package_t test_tx_package,test_tx_package2,test_tx_package3;
uint8_t tail[4]{0x00, 0x00, 0x80, 0x7f};

//////////////////////////////////////////////////////////

uint8_t debug_buffer[9];

Subscriber *sub_debug;
pub_Control_Data debug_pid;

VOFA_Instance_t *vofa_instance = NULL;
Uart_Instance_t *vofa_uart_instance = NULL;
extern uart_package_t VOFA_uart_package;

//////////////////////////////////////////////////
CAN_Tx_Instance_t test_m3508_tx = {
    .can_handle = &hcan1,
    .isExTid = 0,
    .tx_mailbox = 0,
    .tx_id = 0x200,
    .tx_len = 8,
    .can_tx_buff = {0},      
};
CAN_Rx_Instance_t test_m3508_rx = {
    .can_handle = &hcan1,
    .RxHeader = {0},
    .rx_len = 8,
    .rx_id = 0x201,
    .can_rx_buff = {0},
};

Motor_Control_Setting_t test_m3508_motor_ctrl = {
    .motor_controller_setting = {
        .speed_PID = {
            .Kp = 0.01,
            .Ki = 0.00,
            .Kd = 0.00,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .DeadBand = 5,  
            .Output_LPF_RC = 0.9,
            .Derivative_LPF_RC = 0.85,
            .OLS_Order = 1,
            .Improve = OutputFilter | Trapezoid_Intergral | Derivative_On_Measurement | Integral_Limit
         },
         .pid_ref = 0,
    },
    .outer_loop_type = SPEED_LOOP,// 外环控制为速度环
    .inner_loop_type = SPEED_LOOP,// 内环控制为速度环
    .motor_is_reverse_flag = MOTOR_DIRECTION_NORMAL,// 正转
    .motor_working_status = MOTOR_ENABLED,// 使能电机
}; 

Motor_C620 test_m3508[1] = {Motor_C620(1,test_m3508_rx,test_m3508_tx,test_m3508_motor_ctrl,-1)};
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef TEST_SYSTEM_TURNER
extern Motor_C610 m2006[1];
extern Motor_C620 chassis_motor[4];
extern Motor_GM6020 gm6020[1];

#endif

float wheel_v = 0;
float ref = 0;

#ifdef DEBUG_GO1_MOTOR

CAN_Rx_Instance_t go1_rx_instance = {
    .can_handle = &hcan2,
    .RxHeader = {0},
    .rx_len = 8,
    .can_rx_buff = {0},
};

CAN_Tx_Instance_t go1_tx_instance = {
    .can_handle = &hcan2,
    .isExTid = 0,
    .tx_mailbox = 0,
    .tx_len = 8,
    .can_tx_buff = {0},
};

Motor_Control_Setting_t go1_motor_ctrl = {0};
// 较为特殊的go1电机，有些选项不需要配置！
GO_M8010 go1_motor[1]={GO_M8010(0,go1_rx_instance,go1_tx_instance,go1_motor_ctrl,0,-1,3)};
#endif


#ifdef TEST_SYSTEM_TURNER 
uint16_t sample_rate = 5000;
uint16_t frequency = 1;
uint16_t num_samples = 100;
float32_t sine = 0;
float32_t phase_increment = 2 * PI * frequency / sample_rate;
float32_t phase = 0.0f;
float ref_temp = 0;
float speed_aps = 0;
#endif

__attribute((noreturn)) void Debug_Task(void *argument)
{
#ifdef TEST_SYSTEM_TURNER
    int count = 0;
#endif

#ifdef VOFA_TO_DEBUG
    /* vofa设备创建 */
    vofa_uart_instance = Uart_Register(&VOFA_uart_package);
    if(vofa_uart_instance == NULL)
    {
        LOGERROR("vofa uart register failed!");
        vTaskDelete(NULL);
    }
    vofa_instance = VOFA_init(vofa_uart_instance, 10);
    if(vofa_instance == NULL)
    {
        LOGERROR("vofa init failed!");
        vTaskDelete(NULL);
    }
    test_tx_package.uart_handle = vofa_instance->vofa_uart_instance->uart_package.uart_handle;
    test_tx_package.tx_buffer = (uint8_t*)&test_m3508[0].speed_aps;
    test_tx_package.tx_buffer_size = sizeof(float);

    test_tx_package2.uart_handle = vofa_instance->vofa_uart_instance->uart_package.uart_handle;
    test_tx_package2.tx_buffer = tail;
    test_tx_package2.tx_buffer_size = sizeof(tail);

#endif

#ifdef DEBUG_GO1_MOTOR
    int debug = 0;
#endif
    
    Subscriber *sub_y = register_sub("test_y_pub",1);
    publish_data y_data;
    for(;;)
    {

#ifdef VOFA_TO_DEBUG
        vofa_instance->vofa_task(vofa_instance);
        LOGINFO("debug task is running!");
        vofa_instance->vofa_uart_instance->Uart_send(test_tx_package);
        
#endif

#ifdef TEST_SYSTEM_TURNER
        float32_t sine_value = arm_sin_f32(phase);
        sine = sine_value*2000;
        /* 更新相位 */
        phase += phase_increment;
        if(phase >= 2 * PI)
        {
            phase -= 2 * PI;
        }
#ifdef TEST_SYSTEM_M3508
        count++;
        if(count <= 3000)
        {
            chassis_motor[3].Motor_Ctrl(0);
            Motor_SendMsgs(chassis_motor);           
        }
        else if(count>3000 && count<=6000)
        {
            chassis_motor[3].Motor_Ctrl(2000);
            Motor_SendMsgs(chassis_motor);
        }
        else if(count >6000 && count <= 9000){
            chassis_motor[3].Motor_Ctrl(0);
            Motor_SendMsgs(chassis_motor);
        }
        else
        {
            count = 0;
        }
        speed_aps = chassis_motor[3].speed_aps;

#endif

#ifdef TEST_SYSTEM_M2006
        m2006[0].Motor_Ctrl(sine);
        Motor_SendMsgs(m2006);
#endif

#ifdef TEST_SYSTEM_GM6020
        count++;
        if(count <= 3000)
        {
            ref = 0;
            gm6020[0].Motor_Ctrl(0);
            Motor_SendMsgs(gm6020);           
        }
        else if(count>3000 && count<=6000)
        {
            ref = 180;
            gm6020[0].Motor_Ctrl(180);
            Motor_SendMsgs(gm6020);
        }
        else if(count >6000 && count <= 9000){
            ref = 0;
            gm6020[0].Motor_Ctrl(0);
            Motor_SendMsgs(gm6020);
        }
        else
        {
            count = 0;
        }

#endif

#endif

#ifdef DEBUG_GO1_MOTOR
        if(debug <= 5000 || debug >= 10000)
        {
            go1_motor[0].GO_Motor_Speed_Ctrl(5,0.02);
            debug++;
        }
        else
        {
            go1_motor[0].GO_Motor_No_Tarque_Ctrl();
            debug++;
        }
#endif
        y_data = sub_y->getdata(sub_y);
        left_y = *(uint16_t*)y_data.data;
        //test_m3508[0].Motor_Ctrl(left_y);
        
        test_tx_package3.uart_handle = vofa_instance->vofa_uart_instance->uart_package.uart_handle;
        test_tx_package3.tx_buffer = (uint8_t*)&left_y;
        test_tx_package3.tx_buffer_size = sizeof(uint16_t);
        vofa_instance->vofa_uart_instance->Uart_send(test_tx_package3);
        vofa_instance->vofa_uart_instance->Uart_send(test_tx_package2);
        
        test_m3508[0].Motor_Ctrl(3000);
        Motor_SendMsgs(test_m3508);

        

        osDelay(1);
    }


}

