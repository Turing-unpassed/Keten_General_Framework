/**
 * @file Unitree_Go1.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-02
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :      目前打算将go1系列电机统一用一条总线做处理，因为它的数据解析程序比较特别！
 * @versioninfo :
 */
#ifndef UNITREE_GO1_H 
#define UNITREE_GO1_H 

/*----------------------------------include-----------------------------------*/
#include "motor_base.h"
#include "motor_interface.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "com_config.h"
#ifdef __cplusplus


extern "C"{
#endif
/*----------------------------------typedef-----------------------------------*/

/**
 * @brief 扩展帧id 拆解为 高位2bit id + 低位 27bit data
 * （真牛魔奇葩
 */
typedef struct
{
    uint8_t id_of_Extid;
    uint32_t data_of_Extid;
}Extid_Analysis_t;

typedef enum
{
    NORMAL_WORKING = 0,
    OVER_HEATED,// 过热
    OVER_CURRENT,// 过流
    OVER_VOLTAGE,// 过压
    WRONG_ENCODER,// 编码器故障
}GO_M8010_Error_Type_e;


/**
 * @brief 格式化数据结构体
 * 
 */
typedef struct
{
    int16_t T = 0;// 期望关节输出扭矩 unit: N.m
    int16_t W = 0;// 期望关节输出速度 unit: rad/s
    int32_t Pos = 0;// 期望关节输出位置 unit: rad
    int16_t K_P = 0;// 期望关节刚度系数 unit: -1.0-1.0
    int16_t K_W = 0;// 期望关节阻尼系数 unit: -1.0-1.0
}motor_format_ref_data;

typedef struct
{
    float T;// 期望关节的输出力矩 （电机本身的力矩）unit：N.m
    float W;// 期望关节速度（电机本身的速度）（rad/s）
    float Pos;// 期望关节位置（rad）
    float K_P;// 关节刚度系数
    float K_W;// 关节速度系数

}motor_real_ref_data;


/*-----------------------------------macro------------------------------------*/
/* 
    扩展帧组成（从高位到低位）
    指令下发：模块id | 下发指示位 | 数据内容指示位 | 控制模式位 | 目标电机id | 控制模式 | 预留位 | 预留位 

    模块上传：
 */

/* 高位2bits作为id标识符 */
/* 模块id 2bits 故最多有3个CAN转RS485模块 */
#define CAN_To_RS485_Module_ID_0                 (0 << 27)
#define CAN_To_RS485_Module_ID_1                 (1 << 27)
#define CAN_To_RS485_Module_ID_2                 (2 << 27)
#define CAN_To_RS485_Module_ID_3                 (3 << 27)    

/* 指令下发为0 模块上传为1,占1bit */
#define GO1_Send_To_Module                       (0 << 26)
#define GO1_Rec_From_Module                      (1 << 26)

/* 数据内容指示位,占2bits */
#define GO1_Send_Data_Mode                       (0 << 24)
#define GO1_Rec_Data_Mode1                       (1 << 24)
#define GO1_Rec_Data_Mode2                       (2 << 24)

/* ---------------------低位3--------------------- */
/* 控制模式位 占1Byte */
#define GO1_Ctrl_Mode_1                          (10 << 16)
#define GO1_Ctrl_Mode_2                          (11 << 16)
#define GO1_Ctrl_Mode_3                          (12 << 16)
#define GO1_Ctrl_Mode_4                          (13 << 16)

/* ---------------------低位2--------------------- */
/* 预留位 */
#define GO1_Send_Null_1Bits                      (0 << 15)    // 1bit的保留位

/* 电机控制模式 占3bits */
#define GO1_Default_Mode                         (0 << 12)    // 锁定
#define GO1_FOC_Closer_Mode                      (1 << 12)    // FOC闭环
#define GO1_Encoder_calibration                  (2 << 12)    // 编码器校准模式
// 剩下0x3~0x7 全为预留位

/* 目标电机id 总线上最多挂载15个GO1电机（理论上）占4bits */
#define GO1_Motor_ID_0                           (0 << 8)
#define GO1_Motor_ID_1                           (1 << 8)
#define GO1_Motor_ID_2                           (2 << 8)
#define GO1_Motor_ID_3                           (3 << 8)
#define GO1_Motor_ID_4                           (4 << 8)
#define GO1_Motor_ID_5                           (5 << 8)
#define GO1_Motor_ID_6                           (6 << 8)
#define GO1_Motor_ID_7                           (7 << 8)
#define GO1_Motor_ID_8                           (8 << 8)
#define GO1_Motor_ID_9                           (9 << 8)
#define GO1_Motor_ID_10                          (10 << 8)
#define GO1_Motor_ID_11                          (11 << 8)
#define GO1_Motor_ID_12                          (12 << 8)
#define GO1_Motor_ID_13                          (13 << 8)
#define GO1_Motor_ID_14                          (14 << 8)
#define GO1_Motor_ID_15                          (15 << 8)  

/* ---------------------低位1--------------------- */
/* 发送预留位,占1Byte */
#define GO1_Send_Null_1Byte                      (0 << 0)// 1Byte的保留位


/*----------------------------------function----------------------------------*/

uint8_t CAN_To_RS485_Module_ID_Callback(uint8_t module_id);

uint8_t GO_Motor_ID_Callback(uint32_t motor_id);



#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

/**
 * @brief GO_M8010电机类 
 * 
 */
class GO_M8010 : public Motor
{
public:
    GO_M8010(uint8_t id,
              const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
              const Motor_Control_Setting_t& ctrl_config,
              int16_t max_current, uint8_t reduction_ratio,
              uint32_t module_id,uint32_t motor_id) 
            : Motor(id,can_rx_instance,can_tx_instance,ctrl_config,max_current,reduction_ratio)
            ,module_id(module_id),motor_id(motor_id){}
    virtual ~GO_M8010() = default;

    virtual void update_Go1(uint8_t can_rx_data[],uint32_t data_id);

    /* 设置期望值,Kpos & Kspd */
    void Set_Ref_KParam(float ref_kpos,float ref_kspd)
    {
        this->real_ref_data.K_P = ref_kpos;
        this->real_ref_data.K_W = ref_kspd;
    }
    void Set_Ref_CtrlParam(float ref_torque,float ref_speed,float ref_pos)
    {
        // 实际给FOC的指令力矩为： K_P*delta_Pos + K_W*delta_W + T
        this->real_ref_data.T = ref_torque;
        this->real_ref_data.W = ref_speed;
        this->real_ref_data.Pos = ref_pos;
    }

    uint8_t GO_Motor_Standard_Ctrl(float ref_kpos,float ref_kspd,float ref_torque,float ref_speed,float ref_pos);
    uint8_t GO_Motor_SaveResource_Ctrl(float ref_kpos,float ref_kspd,float ref_torque,float ref_speed,float ref_pos);
    uint8_t GO_Motor_ReadBack_Ctrl();
protected:
    /* can帧发送数据打包 */
    virtual void CANMsg_Process()
    {
        /* @todo:对期望真实值进行限幅 */
        motor_constraint(&(this->real_ref_data.K_P),static_cast<float>(0.0f),static_cast<float>(25.599f));// 对期望关节刚度系数限幅
        motor_constraint(&(this->real_ref_data.K_W),static_cast<float>(0.0f),static_cast<float>(25.599f));// 对期望关节阻尼系数限幅
        motor_constraint(&(this->real_ref_data.T),static_cast<float>(-127.99f),static_cast<float>(127.99f));
        motor_constraint(&(this->real_ref_data.W),static_cast<float>(-804.00f),static_cast<float>(804.00f));
        motor_constraint(&(this->real_ref_data.Pos),static_cast<float>(-411774.0f),static_cast<float>(411774.0f));

        this->ref_send_data.K_P = (uint16_t)this->real_ref_data.K_P * 1280.0f;
        this->ref_send_data.K_W = (uint16_t)this->real_ref_data.K_W * 1280.0f;
        this->ref_send_data.T = (uint16_t)this->real_ref_data.T * 256.0f;
        this->ref_send_data.W = (uint16_t)this->real_ref_data.W * 256.0f / (2 * PI);
        this->ref_send_data.Pos = (uint32_t)this->real_ref_data.Pos* 32768 / (2 * PI);
        prepareCANMsg();
    }   
    /* 错误代号解析 */
    void errorType_Get(uint8_t error_id)
    {
        switch(error_id)
        {
            case 0:
                this->error_type = NORMAL_WORKING;
                break;
            case 1:
                this->error_type = OVER_HEATED;
                break;
            case 2:
                this->error_type = OVER_CURRENT;
                break;
            case 3:
                this->error_type = OVER_VOLTAGE;
                break;
            case 4:
                this->error_type = WRONG_ENCODER;
                break;
        }
    }
    void air_pressure_parameters_Get(uint16_t data){    this->air_pressure_parameters = (float)data; }
    void temp_get(uint16_t data){   this->temp = (float)data; }

    /* 设置can发送数据段 */
    void prepareCANMsg()
    {

        if((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_1)
        {
            // 控制模式位为10  @todo:"这个模式下，每发一次就会收到一次电机返回数据"这个怎么实现？
            can_tx_for_motor.can_tx_buff[7] = (uint8_t)((this->ref_send_data.T >> 8) & 0xFF); 
            can_tx_for_motor.can_tx_buff[6] = (uint8_t)(this->ref_send_data.T & 0xFF);
            can_tx_for_motor.can_tx_buff[5] = (uint8_t)((this->ref_send_data.W >> 8) & 0xFF);
            can_tx_for_motor.can_tx_buff[4] = (uint8_t)(this->ref_send_data.W & 0xFF);
            can_tx_for_motor.can_tx_buff[3] = (uint8_t)((this->ref_send_data.Pos >> 24) & 0xFF);
            can_tx_for_motor.can_tx_buff[2] = (uint8_t)((this->ref_send_data.Pos >> 16) & 0xFF);
            can_tx_for_motor.can_tx_buff[1] = (uint8_t)((this->ref_send_data.Pos >> 8) & 0xFF);
            can_tx_for_motor.can_tx_buff[0] = (uint8_t)(this->ref_send_data.Pos & 0xFF);
        }
        else if((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_2)
        {
            // 控制模式位为11 设置 Kpos 和 Kspd
            can_tx_for_motor.can_tx_buff[7] = 0;can_tx_for_motor.can_tx_buff[6] = 0;can_tx_for_motor.can_tx_buff[5] = 0;can_tx_for_motor.can_tx_buff[4] = 0;
            can_tx_for_motor.can_tx_buff[3] = (uint8_t)((this->ref_send_data.K_P >> 8) & 0xFF);
            can_tx_for_motor.can_tx_buff[2] = (uint8_t)(this->ref_send_data.K_P & 0xFF);
            can_tx_for_motor.can_tx_buff[1] = (uint8_t)((this->ref_send_data.K_W >> 8) & 0xFF); 
            can_tx_for_motor.can_tx_buff[0] = (uint8_t)(this->ref_send_data.K_W & 0xFF); 
        }
        else if((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_3)
        {
            // 控制模式位为12
            memset(can_tx_for_motor.can_tx_buff,0,sizeof(can_tx_for_motor.can_tx_buff));// 直接清零发送
        }
        else if((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_4)
        {
            // 控制模式位为13 @todo:这个模式下，发送控制信息除非在电机报错，否则不会有数据返回
            can_tx_for_motor.can_tx_buff[7] = (uint8_t)((this->ref_send_data.T >> 8) & 0xFF); 
            can_tx_for_motor.can_tx_buff[6] = (uint8_t)(this->ref_send_data.T & 0xFF);
            can_tx_for_motor.can_tx_buff[5] = (uint8_t)((this->ref_send_data.W >> 8) & 0xFF);
            can_tx_for_motor.can_tx_buff[4] = (uint8_t)(this->ref_send_data.W & 0xFF);
            can_tx_for_motor.can_tx_buff[3] = (uint8_t)((this->ref_send_data.Pos >> 24) & 0xFF);
            can_tx_for_motor.can_tx_buff[2] = (uint8_t)((this->ref_send_data.Pos >> 16) & 0xFF);
            can_tx_for_motor.can_tx_buff[1] = (uint8_t)((this->ref_send_data.Pos >> 8) & 0xFF);
            can_tx_for_motor.can_tx_buff[0] = (uint8_t)(this->ref_send_data.Pos & 0xFF);
        }
    }
    /* 设置发送的扩展帧id */
    void Set_Send_Extid(uint32_t extid)
    {
        this->can_tx_for_motor.isExTid = true;
        this->can_tx_for_motor.tx_id = extid;
    }

    void processRxMsg();

    Extid_Analysis_t extid_ana = {0};
    /* 期望和当前的真实速度 */
    motor_real_ref_data real_ref_data = {0};
    motor_real_ref_data real_cur_data = {0};
    /* 期望和当前被格式化后的数据 */
    motor_format_ref_data ref_send_data = {0};
    motor_format_ref_data cur_rec_data = {0};

    GO_M8010_Error_Type_e error_type = NORMAL_WORKING;// 电机错误标识: 0.正常 1.过热 2.过流 3.过压 4.编码器故障 5-7.保留
    float air_pressure_parameters = 0;
    int temp;

private:
    uint32_t module_id = 0;
    uint32_t motor_id = 0;
    float last_ref_kpos = 0;
    float last_ref_kspd = 0;

public:
    /* 这里是不适合原父类中虚函数的实现，不写会报错，暂时没想到什么优雅的实现方法，反正这下面的不用管就是了 */
    virtual void update(uint8_t can_rx_data[]){}
    virtual void CanMsg_Process(CAN_Tx_Instance_t &can_txmsg){}
    virtual void set_motor_ref(float ref){};
    virtual void stop_the_motor(){};
    virtual void enable_the_motor(){};
    virtual void pid_control_to_motor(){};
};







/*----------------------------------variable----------------------------------*/


#endif


#endif	/* UNITREE_GO1_H */
