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
/* module层接口 */
#include "soft_iwdg.h"
#include "bsp_can.h"
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


uint8_t Chassis_Control();


/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "rm_motor.h"
#endif




