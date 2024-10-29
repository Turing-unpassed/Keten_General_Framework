/**
 * @file air_joy.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef AIR_JOY_H 
#define AIR_JOY_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
/* bsp层接口 */
#include "bsp_dwt.h"
#include "bsp_gpio.h"

/* module层接口 */
#include "topics.h"
#include "data_type.h"
#include <math.h>
/*-----------------------------------macro------------------------------------*/
#define MAX_ACCELERATION 0.5f
#define MAX_VELOCITY 2.7f

/*----------------------------------typedef-----------------------------------*/

typedef enum
{
    NORMAL = 0,// 遥杆数据直驱
    TRAPEZOIDAL,// 遥杆数据梯形规划处理
}Process_method_e;

typedef struct 
{
    float current_velocity;
    float target_velocity;
}TrapezoidalState;

typedef enum
{
    NONE_EVENT = 0x00,
    SWA_EVENT = 0x01,// swa 被置位
    SWB_EVENT = 0x02,// swb 被置位
    SWC_EVENT = 0x04,// swc 被置位
    SWD_EVENT = 0x08,// swd 被置位
}SWO_EVENT_e;

typedef struct
{
    GPIO_Instance_t *air_joy_gpio;
    uint16_t SWA,SWB,SWC,SWD;
    uint16_t LEFT_X,LEFT_Y,RIGHT_X,RIGHT_Y;

    uint32_t last_ppm_time,now_ppm_time;
    uint16_t PPM_Buf[10];
    uint8_t ppm_ready;
    uint8_t ppm_sample_cnt;
    uint8_t ppm_update_flag;
    uint16_t ppm_time_delta;

    uint16_t last_swo_buf[4];
    uint8_t swo_event;
    /* 控制模式 */
    Process_method_e process_method;

    /* 发布控制信息，发布者注册 */
    Publisher *air_joy_pub;
    pub_Control_Data control_data;
}Air_Joy_Instance_t;

/*----------------------------------variable----------------------------------*/
extern Air_Joy_Instance_t *air_instance;


/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @param gpio_instance 
 * @param method abs_diff
 * @return uint8_t 
 */
uint8_t Air_Joy_Init(GPIO_Instance_t *gpio_instance,Process_method_e method);


/**
 * @brief 
 * 
 * @param instance 
 * @return uint8_t 
 */
void Air_Update(void *instance);


/**
 * @brief 航模遥控数据处理函数
 * 
 * @return uint8_t 
 * @attention 这里尤为注意：航模遥控输出的X、Y 是传统的 前Y 右X 坐标系
 *                  2000
 *             1000 1500 2000
 *                  1000
 *            而机器人实际使用坐标系为 前X 左Y 的通用机器人坐标系，所以在这里面直接对航模遥控的输出做特定处理，就没有特别说再解耦出坐标转换器
 *            然后经过坐标转换之后再给到发布速度
 *            （当然这里只是我懒得搞了，以后你们可以试着解耦doge
 */
void Air_Joy_Process();


uint8_t Air_Joy_Publish();

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif	/* AIR_JOY_H */
