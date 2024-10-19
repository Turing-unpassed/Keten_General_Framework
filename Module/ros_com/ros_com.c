/**
 * @file ros_com.c
 * @author Keten (2863861004@qq.com)
 * @brief ros和32通讯包解析，解析之后数据通过话题发布出去
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "ros_com.h"

static uint8_t ROSCom_Rtos_init(ROS_Com_Instance_t* ros_instance,uint32_t queue_length);

uint8_t ros_uart_buffer[51];

ROS_Com_Instance_t *ros_instance;

uart_package_t ros_uart_package = {
    .uart_handle = &huart1,
    .rx_buffer = ros_uart_buffer,
    .rx_buffer_size = 51,
    .uart_callback = ROSCom_RxCallback_Fun,
};

Uart_Instance_t *ros_uart_instance;

iwdg_config_t ros_iwdg_config = {
    .reload_count = 1000,
    .init_count = 0,
    .callback = IWDG_For_ROSCOM_Rx,
};

IWDG_Instance_t *ros_iwdg_instance;

union roscom
{
    uint8_t data[6*4+2+2+1];// 6个float + 2个帧头 + 2个帧尾
    float value[6];// 6个数据位
}roscom_data;

uint8_t ROS_Communication_Init()
{
    /* 动态创建 */
    ros_instance = (ROS_Com_Instance_t*)pvPortMalloc(sizeof(ROS_Com_Instance_t));
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance malloc failed!");
        return 0;
    }

    /* 挂载uart串口接口 */
    ros_uart_instance = Uart_Register(&ros_uart_package);
    if(ros_uart_instance == NULL)
    {
        LOGERROR("ros uart instance init failed!");
        return 0;
    }
    ros_instance->uart_instance = ros_uart_instance;

    /* 挂载iwdg接口 */    
    ros_iwdg_instance = IWDG_Register(&ros_iwdg_config);
    if(ros_iwdg_instance == NULL)
    {
        LOGERROR("ros iwdg instance init failed!");
        vPortFree(ros_instance);
        ros_instance = NULL;
        return 0;
    }
    ros_instance->iwdg_instance = ros_iwdg_instance;

    /* rtos api ini */
    if(ROSCom_Rtos_init(ros_instance,ROSCOM_QUEUE_LENGTH) != 1)
    {
        LOGERROR("ros rtos init failed!");
        vPortFree(ros_instance);
        ros_instance = NULL;
        return 0;
    }

    /* 挂载内部接口 */
    ros_instance->get_data = ROS_GetData;
    ros_instance->roscom_task = ROSCOM_Task;
    ros_instance->roscom_deinit = ROSCOM_DeInit;

    /* 初始化成功 */
    return 1;
}


static uint8_t ROSCom_Rtos_init(ROS_Com_Instance_t* ros_instance,uint32_t queue_length)
{
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance rtos init failed!");
        return 0;
    }
    rtos_for_module_t *rtos_for_roscom = (rtos_for_module_t *)pvPortMalloc(sizeof(rtos_for_module_t));
    if(rtos_for_roscom == NULL)
    {
        LOGERROR("rtos_for_roscom malloc failed!");
        return 0;
    }
    memset(rtos_for_roscom,0,sizeof(rtos_for_module_t));

    rtos_for_roscom->queue_send = queue_send_wrapper;
    rtos_for_roscom->queue_receive = xQueueReceive;

    QueueHandle_t queue = xQueueCreate(queue_length,sizeof(UART_TxMsg));
    if(queue == NULL)
    {
        LOGERROR("queue create failed!");
        vPortFree(rtos_for_roscom);
        rtos_for_roscom = NULL;
        return 0;
    }
    else
    {
        rtos_for_roscom->xQueue = queue;
    }

    ros_instance->rtos_for_roscom = rtos_for_roscom;  
    LOGINFO("ros rtos init success!");
    return 1;
}

uint8_t ROS_GetData(uint8_t *data,pub_ros_package *ros_package)
{
    if(data == NULL)
    {
        LOGERROR("ros data is NULL!");
        return 0;
    }
    return 1;
}

uint8_t ROSCom_RxCallback_Fun(void *uart_instance, uint16_t data_len)
{
    UART_TxMsg Msg;
    if(uart_instance == NULL)
    {
        LOGERROR("uart instance is NULL");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = (Uart_Instance_t*)uart_instance;
    ROS_Com_Instance_t *temp_ros_instance = temp_uart_instance->device;

    if(temp_ros_instance->rtos_for_roscom->xQueue != NULL && temp_ros_instance->rtos_for_roscom->queue_send != NULL)
    {
        Msg.data_addr = temp_ros_instance->uart_instance->uart_package.rx_buffer;
        Msg.len = data_len;
        Msg.huart = temp_ros_instance->uart_instance->uart_package.uart_handle;
        if(Msg.data_addr != NULL)
        {
            temp_ros_instance->rtos_for_roscom->queue_send(temp_ros_instance->rtos_for_roscom->xQueue,&Msg,NULL);
            return 1;
        }
    }

    return 0;
}

uint8_t ROSCOM_Task(void *ros_instance)
{
    UART_TxMsg Msg;
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance is NULL!");
        return 0;
    }
    ROS_Com_Instance_t *temp_ros_instance = ros_instance;
    if(temp_ros_instance->rtos_for_roscom->xQueue != NULL && temp_ros_instance->rtos_for_roscom->queue_receive != NULL)
    {
        if(temp_ros_instance->rtos_for_roscom->queue_receive(temp_ros_instance->rtos_for_roscom->xQueue,&Msg,0) == pdPASS)
        {
            if(ROS_GetData(Msg.data_addr,&temp_ros_instance->ros_package) == 1)
            {
                publish_data temp_data;
                temp_data.data = (uint8_t*)&temp_ros_instance->ros_package;
                temp_data.len = sizeof(pub_ros_package);
                temp_ros_instance->ros_pub->publish(temp_ros_instance->ros_pub,temp_data);
                return 1;
            }
        }
    }
    return 0;
}


uint8_t IWDG_For_ROSCOM_Rx(void *device)
{
    if(device == NULL)
    {
        LOGERROR("device is NULL!");
        return 0;
    }

    /* 在这里做相应没喂狗的处理，比如使用蜂鸣器警告，然后stm32重新打开串口的处理 */
    return 1;
}


uint8_t ROSCOM_DeInit(void *ros_instance)
{
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance is NULL!");
        return 0;
    }
    ROS_Com_Instance_t *temp_ros_instance = ros_instance;

    if(temp_ros_instance->rtos_for_roscom != NULL)
    {
        temp_ros_instance->rtos_for_roscom->queue_receive = NULL;
        temp_ros_instance->rtos_for_roscom->queue_send = NULL;
        if(temp_ros_instance->rtos_for_roscom->xQueue != NULL)
        {
            vQueueDelete(temp_ros_instance->rtos_for_roscom->xQueue);
            temp_ros_instance->rtos_for_roscom->xQueue = NULL;
        }
        vPortFree(temp_ros_instance->rtos_for_roscom);
        temp_ros_instance->rtos_for_roscom = NULL;
    }

    /* 清除roscom中串口接口 */
    if(temp_ros_instance->uart_instance != NULL)
    {
        temp_ros_instance->uart_instance->Uart_Deinit(temp_ros_instance->uart_instance);
    }

    temp_ros_instance->get_data = NULL;
    temp_ros_instance->roscom_task = NULL;
    temp_ros_instance->roscom_deinit = NULL;

    vPortFree(temp_ros_instance);
    temp_ros_instance = NULL;
    ros_instance = NULL;
    return 1;
}


