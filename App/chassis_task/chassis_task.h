/**
 * @file chassis_task.h
 * @author Keten (2863861004@qq.com)
 * @brief 底盘任务函数
 *        实现需求：注册底盘用的电机，并且为每一个电机挂载看门狗监测线程
 *                 丰富底盘类型，目前打算加入全向轮系列底盘、舵轮底盘
 *                 底盘任务需要结合控制器线程，只有控制器线程准备好了，底盘线程才会发送控制信息到电机
 *                 
 * @version 0.1
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/

#include <stdint.h>
/* freertos接口 */
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* bsp层接口 */
#include "bsp_can.h"
/* module层接口 */
#include "soft_iwdg.h"
#include "topics.h"
#include "ccmram.h"
/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Chassis_Init();


void Chassis_Task(void *argument);


/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif
  
#ifdef __cplusplus
#include "rm_motor.h"
#include "Omni/Omni_Chassis.h"


/**
 * @brief Chassis底盘运动函数
 *        完成功能：
 *          1.订阅姿态数据
 *          2.底盘模式switch-case
 *          3.速度命令解算得到各个轮子的速度
 *          4.将轮子速度发送到各自电机 
 * @param user_chassis 
 * @return uint8_t 
 */
uint8_t Chassis();




#endif




