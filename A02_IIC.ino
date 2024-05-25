/***********************************************************************
 * Copyright(C) 2021 深圳市电应普科技
 * 文件  : A02_IIC.ino
 * 描述 :   定时触发传感器工作,解析传感器输出数据(IIC协议数据)打印到 PC 串口
 *          arduino unon 开发板作为IIC主机向从机的0x02寄存器写入0xB4触发传感器工作,
 *          等待60MS后读取0x03寄存器中距离值（两个字节），详细协议说明请阅读A02模组IIC相关规格书
 *          成功接收到传感器回复的距离数据后，将距离值输出到 PC 串口
 *          此例程仅在 arduino unon 开发板上测试通过,包含Wire一个第 3 方库,请自行添加
 *          A4 连接传感器 TX 引脚,A5 连接传感器 RX 引脚
 * 版本与日期  作者  说明
 * 211130-A0   YYL  创建了该文件
 ***********************************************************************/
/* 包含头文件 -------------------------------------------------------------------*/
#include <Arduino.h>
#include <Wire.h> 

/* 宏定义 -----------------------------------------------------------------------*/
#define SOFT_VERSION "A02_IIC-211130-A0"  //软件版本

/* 类型定义 ---------------------------------------------------------------------*/
/* 扩展变量 ---------------------------------------------------------------------*/
/* 函数声明 ---------------------------------------------------------------------*/
void digital_toggle( uint8_t pin );
/* 私有变量 ---------------------------------------------------------------------*/
/* 函数实现 ---------------------------------------------------------------------*/
void digital_toggle( uint8_t pin )
{
    digitalRead( pin ) ? digitalWrite( pin, 0 ) : digitalWrite( pin, 1 );
}

void setup()
{
    // put your setup code here, to run once:
    pinMode( LED_BUILTIN, OUTPUT );//初始化LED
    Serial.begin( 9600 );//初始化主串口 波特率9600
    Serial.println( SOFT_VERSION );//主串口输出软件版本
    Wire.begin();//以主机形式加入IIC总线
}

void loop()
{
    static uint32_t trigger_cnt = 0;
    static uint8_t  recv_buf[ 10 ] = { 0 };
    static uint16_t len = 0;    
    static uint8_t trigger_flag = 0; 

    if ( millis() - trigger_cnt > 500 )//以500ms的周期发送触发传感器工作指令
    {     
        Wire.beginTransmission(0x74);//开始向0X74从机写入数据 
        Wire.write(0x02);//寄存器0x02
        Wire.write(0xB4);//写入指令数据0xB4  
        Wire.endTransmission();//停止与从机的数据传输
        trigger_flag = 1;//触发工作标志置1
        trigger_cnt = millis();  
    }
    if(trigger_flag)//触发工作标志置1
    {      
       if ( millis() - trigger_cnt > 60 )//触发后等待60MS再读取距离值
       {
           trigger_flag = 0;//触发工作标志清零
           Wire.beginTransmission(0x74);//开始向0X74从机写入数据   
           Wire.write(0x03);//寄存器0x03
           Wire.endTransmission(0);//停止与从机的数据传输，但不发送stop信号        
           Wire.requestFrom(0x74, 2);//向0x74从机请求2个字节数据
           while(Wire.available()) //如果Wire上有 char 等读取
           {
                recv_buf[len++] = Wire.read(); // 从Wire 上读取一个char
           }
           if ( len >= 2 )//当模拟串口接收到的数据>=3个字节
            {
                len = 0;               
                uint16_t distance   = recv_buf[ 0 ] << 8 | recv_buf[ 1 ];//获取距离值
                char     out_txt[ 32 ] = { 0 };           
                sprintf( out_txt,"distance : %d mm \r\n", distance );          
                Serial.print( out_txt );//输出距离值到主串口               
            }                     
       }
    }

    static uint32_t led_cnt = millis();
    if ( millis() - led_cnt > 100 )//LED以100ms周期闪烁
    {
        digital_toggle( LED_BUILTIN );
        led_cnt = millis();
    }
}
// main.cpp
