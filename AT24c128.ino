/*-----------------------------------------------
* 选择Atmega328P控制器，片外16Mhz晶体振荡器，AT24C128,A0,A1接地处理
WP接地处理，器件读地址为0xA0。串口波特率9600，无校验。
* 设计：希岩（凌晨七點半）
* 时间：2017.10.31
*
--------------------------------------------------*/

#define AT24C_Add_W  0xA0   //器件读地址，A1,A0=0， 默认选择0~255字节地址
#define AT24C_Add_R  0xA1   //器件写地址，A1,A0=0， 默认选择0~255字节地址
#define SLAW         0x18   //模块正确地址应答常量写 page200
#define DataOKW      0x28   //模块正确数据写应答常量
#define SLAR         0x40   //地址收到ok,page204
#define DataOKR      0x50   //数据收到ok
#define WDR()      MCUSR &= ~(1<<WDRF)   //清看门狗

//#define WRITE

//数据类型宏定义
typedef unsigned char uint08;
typedef signed   char sint08;
typedef unsigned int  uint16;
typedef signed   int  sint16;

//发送数据缓存区，ASCII码形式，不支持中文,注意在字符串尾加0xFF作为结束标记
uint08 Send_Buff[]="Hello World!\x0D\x0AI am so glad you can see it.\x0D\x0AYou are the most beautiful sight I have ever seen.\x0D\x0A\xFF";   
uint08 Read_Buff[100]={0};                   //数据接收缓存区，AT24C128最大空间128k字节


/*-------------------------------初始化-----------------------------------*/
//------------------------------------------------------------------------
//AT24时钟频率1Mhz,设置单片机比特率
//SCL frequency=cpu clock/(16+2(TWBR)*(prescalerValue))
void Init_TWI(void)
{ TWCR = 0x00;                       //中止I2C
  PRR =  0x00;                       //复位功耗抑制寄存器，TWI唤醒
  TWBR =   24;                       //比特率寄存器为24，产生9615波特率
  TWSR|= 0x02;                      //比特率预分频因子16，page194
  //TWAR=0xFF;                     //工作于主机模式不需要此地址
  TWCR = (1<<TWEN);                 //TWI使能,TWINF写1清零
  
}

//初始化看门狗------------------------------------------------------------------
void Init_watchdog(void)
{
  WDR();                        //清看门狗
  WDTCSR = 0x00;                 //看门狗定时器禁用 page46                  
}
//----------------------------------初始化控制器---------------------------------   
void Init_devices(void)
{  
  Init_watchdog();
  Init_TWI();

}
//------------------------------------------------------------------------
//启动I2C
void TWI_Start(void)                           //refer to page198
{  
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);   //TWI start，主机模式，page149
    while(!(TWCR&(1<<TWINT))) ;                 //等待TWINF置位以及收到应答信号
}
//-------------------------------------------------------------------------------
//主机发送一个字节
void TWI_Write(uint08 str)
{
   TWDR = str;                               //将字符写入数据寄存器
   TWCR = (1<<TWINT)|(1<<TWEN);              //启动发送地址及数据，page198
   while(!(TWCR&(1<<TWINT)));               //等待TWINF置位，SLA+W或data已发出

}
//-------------------------------------------------------------------------------
//总线读出一个字符返回读出的字符
uint08 TWI_READ(void)
{  TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
   while(!(TWCR&(1<<TWINT)));
   return(TWDR);                            //返回读出的数据
}
//-------------------------------------------------------------------
//I2C应答函数
#define TWI_ACK()  (TWSR&0xF8)                  //返回TWI状态，高5位
//--------------------------------------------------------------------------------
//I2C 停止
#define TWI_Stop()  TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);   //page198 datasheet

//---------------------------------------------------------------------------------
//向I2C slave写入数据,第一个参数是页选择为0和1。第二个是字节地址0xFF，第三个参数为字节数据
void AT24_TWI_Write(uint08 ByteAddH,uint08 ByteAddL,uint08 Str)
{ TWI_Start();                                 //开启发送     
  TWI_Write(AT24C_Add_W);               //发送从机地址
  if(TWI_ACK()==SLAW)                        //应答
   TWI_Write(ByteAddH);                       //发送字节地址
  if(TWI_ACK()==DataOKW)                     //数据应答
   TWI_Write(ByteAddL);                       //发送字节地址
  if(TWI_ACK()==DataOKW)                     //数据应答
   TWI_Write(Str);                          //发送数据
   TWI_ACK();                               //写入数据应答
   TWI_Stop();                             //发送停止信号
   delay(5);                               //延时5ms,完成写入
}
  
//--------------------------------------------------------------------------------
//从I2C读取数据参数1为变量地址，参数2为页选择，参数3为字节地址，参数4为读取数量
//读取模式为随机读
uint08 AT24_TWI_Read(uint08 ByteAddH,uint08 ByteAddL)
{ uint08 u08temp;
  TWI_Start();                                  //发送起始信号                     
  TWI_Write(AT24C_Add_W);                     //写入地址及页选择位
  if(TWI_ACK()==SLAW)                         //地址发送应答
    TWI_Write(ByteAddH);                       //写入字节高地址
  if(TWI_ACK()==DataOKW)                      //发送页码和字节地址
    TWI_Write(ByteAddL);                       //写入字节低地址
  if(TWI_ACK()==DataOKW)                      //发送页码和字节地址
   TWI_Start();                                //再一次开始
  TWI_Write(AT24C_Add_R);                     //写入读地址及页选择位
  if(TWI_ACK()==SLAR)                        //读应答,返回值有误
   u08temp=TWI_READ();                         //从总线读一个字节存入中间量
   TWI_ACK();                                   //应答
  TWI_Stop();                                 //发送停止信号   
  return u08temp;                             //返回读取的数据
}
//--------------------------------------------------------------------------------
//读取模式为读取当前位置数据
uint08 AT24_CUR_Read(void)
{ uint08 u08temp;
  TWI_Start();                                  //发送起始信号                     
  TWI_Write(AT24C_Add_R);                  //写入读地址及页选择位
  if(TWI_ACK()==SLAR) ;                       //读应答,返回值有误
  TWI_ACK();
   u08temp=TWI_READ();                         //从总线读一个字节存入中间量
  TWI_ACK();                                   //应答
  TWI_Stop();                                 //发送停止信号   
  return u08temp;                             //返回读取的数据
}

//------------------------------------------------------------------
//写入数据函数，当前最多写入256个数据
void Write_AT24(uint16 ByteAdd,uint08 ch[])
{ uint16 u16i;
  uint16 u16n=0;
  uint08 u8adddH,u8adddL;
  u16n=strlen(ch);                               //字符长度
  for(u16i=0;u16i<u16n;u16i++,ByteAdd++)
  {  u8adddH=(ByteAdd>>8)&0xff;
     u8adddL=(uint08)ByteAdd;
     AT24_TWI_Write(u8adddH,u8adddL,ch[u16i]);  //向AT24C04写入数据
     delay(5);                                  //延时5ms
   
  }
}
//------------------------------------------------------------------
//从指定位置读入数据函数，当前最多读取256个数据
void Read_At24(uint08 *pchar,uint16 ByteAdd,uint08 u8len)
{uint08 u08i;
uint08 u8adddH,u8adddL;

for(u08i=0;u08i<u8len;u08i++,ByteAdd++)                                    
{
  u8adddH=(ByteAdd>>8)&0xff;
  u8adddL=(uint08)ByteAdd;
   *(pchar+u08i)=AT24_TWI_Read(u8adddH,u8adddL);  //读一个字节，写入缓冲区

}
}
//------------------------------------------------------------------
//增量式读入数据函数,当前最多读取256个数据
void Read_Increm(uint08 *pchar,uint16 ByteAdd,uint08 u8len)
{uint08 u08i,u08tempt;
uint08 u8adddH,u8adddL;
  //第一个字节定位
  u8adddH=(ByteAdd>>8)&0xff;
  u8adddL=(uint08)ByteAdd;
  *(pchar++)=AT24_TWI_Read(u8adddH,u8adddL);  //读一个字节

for(u08i=0;u08i<u8len;u08i++,ByteAdd++)                                    
   *(pchar+u08i)=AT24_CUR_Read();

}
//--------------------------------------------------------------
//USART发送数据
void USART_Transmit(uint08 data)
{
  while(!(UCSR0A&(1<<UDRE0)));   //等待发送缓冲器为空
  UDR0=data;                     //将数据放入缓冲器，发送数据
}
//-------------------------------------
//#define WRITE           //条件编译
void setup() {
  
  pinMode(13,OUTPUT);
  uint08 u08i,u08Tempt;
  Serial.begin(9600);
  Init_devices();
  cli();                       //清中断
  Init_devices();              //初始化设备
  sei();                       //中断使能
  #ifdef WRITE
   Write_AT24(0,Send_Buff);                                   //写数据
   Serial.println("Write OK.");
#else
  Read_At24(Read_Buff,0,100);                                   //指定位置读数据
  //Read_Increm(Read_Buff,0,100);                                 //增量式读取数据
  
   //将读出的数据发送到串口
  for(u08i=0;u08i<100;u08i++)
   {u08Tempt=Read_Buff[u08i];
   if(u08Tempt!=0xFF)                                      //EEPROM空字节为0xFF
     USART_Transmit(u08Tempt);                             //发送读到的数据
    else break;
   }
   Serial.println("Read OK.");
#endif


}
//-------------------------------------
void loop()
{
digitalWrite(13,1);
delay(1000);
digitalWrite(13,0);
delay(1000);
  
}
