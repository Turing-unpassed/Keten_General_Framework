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
 * @note :      2024-11-03 修改CAN2接收回调函数，使其适配宇树go1 电机的数据解析
 *                         移除了TEST_SYSTEM_M2006的测试
 * @versioninfo :
 */
#include "com_config.h"
#include "topics.h"

extern Motor_C610 m2006[1];

extern GO_M8010 go1_motor[1];

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

<<<<<<< HEAD
// 现在打算can2专控go1电机，当然也可以换上别的电机，只是暂时先用
void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    uint32_t data_of_id = (uint32_t)can_instance->RxHeader.ExtId & 0x07FFFFFF;
    uint8_t temp_module_id = CAN_To_RS485_Module_ID_Callback((uint8_t)(can_instance->RxHeader.ExtId >> 27) & 0x03);// 解析出标识符（模块id）
    uint8_t temp_motor_id = GO_Motor_ID_Callback(data_of_id);// 解析出扩展帧中的数据部分

=======
float aaa = 0;
// 现在打算can2专控go1电机，当然也可以换上别的电机，只是暂时先用
void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    aaa+=0.01;
    Extid_Analysis_t temp_ana;
    temp_ana.id_of_Extid = (uint8_t)(can_instance->RxHeader.ExtId >> 28) & 0x03;
    temp_ana.data_of_Extid = (uint32_t)can_instance->RxHeader.ExtId & 0x07FFFFFF;
    uint8_t temp_module_id = CAN_To_RS485_Module_ID_Callback(temp_ana.id_of_Extid);
    uint8_t temp_motor_id = GO_Motor_ID_Callback(temp_ana.data_of_Extid);
>>>>>>> ed5a7bc63a58a0dc3d0b3d2590c344075653ba47
    if(temp_motor_id < 0)
    {
        return;
    }
    switch(temp_module_id)
    {
<<<<<<< HEAD
        case 0:
=======
        case 0:// 模块id为0
            go1_motor[temp_motor_id].update_Go1(can_instance->can_rx_buff,temp_ana.data_of_Extid);
>>>>>>> ed5a7bc63a58a0dc3d0b3d2590c344075653ba47
            break;
        case 1:
            break;
        case 2:
            break;
<<<<<<< HEAD
        case 3:// 模块出厂id为3
            go1_motor[temp_motor_id].update_Go1(can_instance->can_rx_buff,data_of_id);
=======
        case 3:
>>>>>>> ed5a7bc63a58a0dc3d0b3d2590c344075653ba47
            break;
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

float ccc = 0;
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
<<<<<<< HEAD
            if(temp_can_txmsg.isExTid == 1)     // 发送扩展帧
=======
            if(temp_can_txmsg.isExTid == 1)
            {// 发送扩展帧
>>>>>>> ed5a7bc63a58a0dc3d0b3d2590c344075653ba47
                CAN_Transmit_ExtId(&temp_can_txmsg);
                ccc+=0.001;
            }
            else    // 发送标准帧
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1000);
    }
}



