/**
 * @file com_config.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-05
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "com_config.h"
#include "topics.h"

extern Motor_C610 m2006[1];

osThreadId_t CAN1_Send_TaskHandle;
osThreadId_t CAN2_Send_TaskHandle;

QueueHandle_t CAN1_TxPort;
QueueHandle_t CAN2_TxPort;


extern Motor_GM6020 gm6020[1];

uint8_t Common_Service_Init()
{
    CAN1_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    CAN2_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    SubPub_Init();// 话题订阅机制开启

    return 1;
}

float motor_acceleration = 0;
float angle = 0;
float motor_current = 0;
void CAN1_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
            case 0x201:
            {
                chassis_motor[0].update(can_instance->can_rx_buff);
                break;
            }
            case 0x202:
            {
                chassis_motor[1].update(can_instance->can_rx_buff);
                break;
            }
            case 0x203:
            {
                chassis_motor[2].update(can_instance->can_rx_buff);
                break;
            }
            case 0x204:
            {
                chassis_motor[3].update(can_instance->can_rx_buff);
                break;
            }
#ifdef TEST_SYSTEM_GM6020
            case 0x205:
            {
                gm6020[0].update(can_instance->can_rx_buff);
                angle = gm6020[0].angle;
                break;
            }
#endif
        }
    }
    if(can_instance->RxHeader.IDE == CAN_ID_EXT)
    {
        switch(can_instance->RxHeader.ExtId)
        {
            case 0x201:
            {
                break;
            }
        }
    }
}


void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
            case 0x201:
            {
#ifdef TEST_SYSTEM_M2006
                m2006[0].update(can_instance->can_rx_buff);
                speed_aps = m2006[0].speed_aps;
                motor_acceleration = m2006[0].motor_acceleration;
#endif
                break;
            }
            case 0x205:
            {
                break;
            }
        }
    }
}


__attribute((noreturn)) void CAN1_Send_Task(void *argument)
{
    CAN_Tx_Instance_t temp_can_txmsg;
    uint8_t free_can_mailbox;
    for(;;)
    {
        if(xQueueReceive(CAN1_TxPort,&temp_can_txmsg,0) == pdTRUE)
        {
            do{
                free_can_mailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
            }while(free_can_mailbox == 0);
            if(temp_can_txmsg.isExTid == 1)// 发送扩展帧
                CAN_Transmit_ExtId(&temp_can_txmsg);
            else    // 发送标准帧
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1);
    }
}


__attribute((noreturn)) void CAN2_Send_Task(void *argument)
{
    CAN_Tx_Instance_t temp_can_txmsg;
    uint8_t free_can_mailbox;
    for(;;)
    {
        if(xQueueReceive(CAN2_TxPort,&temp_can_txmsg,0) == pdTRUE)
        {
            do{
                free_can_mailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan2);
            }while(free_can_mailbox == 0);
            if(temp_can_txmsg.isExTid == 1)// 发送扩展帧
                CAN_Transmit_ExtId(&temp_can_txmsg);
            else    // 发送标准帧
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1);
    }
}



