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

uint8_t debug_buffer[9];

Subscriber *sub_debug;
pub_Control_Data debug_pid;

VOFA_Instance_t *vofa_instance = NULL;
Uart_Instance_t *vofa_uart_instance = NULL;
extern uart_package_t VOFA_uart_package;

#ifdef TEST_SYSTEM_TURNER
extern Motor_C610 m2006[1];
extern Motor_C620 chassis_motor[4];
extern Motor_GM6020 gm6020[1];

#endif


extern Omni_Chassis User_Chassis;


float wheel_v = 0;
float ref = 0;
#ifdef TEST_SYSTEM_TURNER 
uint16_t sample_rate = 5000;
uint16_t frequency = 1;
uint16_t num_samples = 100;
float32_t sine = 0;
float32_t phase_increment = 2 * PI * frequency / sample_rate;
float32_t phase = 0.0f;
float ref_temp = 0;
float speed_aps = 0;
float Dout = 0;
float Iout = 0;
float Pout = 0;
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
#endif
    for(;;)
    {

#ifdef VOFA_TO_DEBUG
        vofa_instance->vofa_task(vofa_instance);
        LOGINFO("debug task is running!");
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
        // chassis_motor[0].Motor_Ctrl(ref_temp);
        // Motor_SendMsgs(chassis_motor);
        // chassis_motor[0].Motor_Ctrl(sine);
        // Motor_SendMsgs(chassis_motor);
        // chassis_motor[1].Motor_Ctrl(sine);
        speed_aps = chassis_motor[3].speed_aps;
        Dout = chassis_motor[3].ctrl_motor_config.motor_controller_setting.speed_PID.Dout;
        Iout = chassis_motor[3].ctrl_motor_config.motor_controller_setting.speed_PID.Iout;
        Pout = chassis_motor[3].ctrl_motor_config.motor_controller_setting.speed_PID.Pout;
#endif

#ifdef TEST_SYSTEM_M2006
        // count++;
        // if(count <= 3000)
        // {
        //     m2006[0].Motor_Ctrl(0);
        //     Motor_SendMsgs(m2006);           
        // }
        // else if(count>3000 && count<=6000)
        // {
        //     m2006[0].Motor_Ctrl(3000);
        //     Motor_SendMsgs(m2006);
        // }
        // else if(count >6000 && count <= 9000){
        //     m2006[0].Motor_Ctrl(0);
        //     Motor_SendMsgs(m2006);
        // }
        // else
        // {
        //     count = 0;
        // }
        // m2006[0].Motor_Ctrl(ref_temp);
        // Motor_SendMsgs(m2006);
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


        // chassis_motor[0].Motor_Ctrl(0);
        // Motor_SendMsgs(chassis_motor);    
#endif
        // User_Chassis.Ref_RoboSpeed.linear_x = 2.5;
        // User_Chassis.Ref_RoboSpeed.linear_y = 0;
        // User_Chassis.Ref_RoboSpeed.omega = 0;
        osDelay(1);
    }
}
