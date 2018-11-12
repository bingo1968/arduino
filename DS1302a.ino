/*
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 8
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
*/
#include <LiquidCrystal.h>

#define SCLK 7
#define DIO 6
#define CE 5

#define SEC_REG  0
#define MIN_REG 1
#define HUR_REG 2
#define DAT_REG 3
#define MTH_REG 4
#define YER_REG 6
#define DAY_REG 5
#define WP_REG   7

typedef uint8_t reg_t;
//  LCD config
const int rs = 12, en = 11, d4 = 8, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//

void writeout(uint8_t value)
{
  //Serial.write(value);
  pinMode(DIO,OUTPUT);
  shiftOut(DIO,SCLK,LSBFIRST,value);
  }

uint8_t readin()
{
  uint8_t input_value = 0;
  uint8_t bit = 0;
  pinMode(DIO,INPUT);
  for(int i=0; i<8; i++){
    bit=digitalRead(DIO);
    input_value|=(bit<<i);
    digitalWrite(SCLK,HIGH);
    delayMicroseconds(1);
    digitalWrite(SCLK,LOW);
    }
    return input_value;
  }

void writeregister(reg_t reg, uint8_t value)
{
  uint8_t cmd_byte = (128 | (reg<<1));
  digitalWrite(SCLK,LOW);
  digitalWrite(CE,HIGH);
  writeout(cmd_byte);
  writeout(value);
  digitalWrite(CE,LOW);
  }

uint8_t readregister(reg_t reg)
{
  uint8_t cmd_byte = 0x81;
  uint8_t reg_value;
  cmd_byte |=(reg<<1);
  digitalWrite(SCLK,LOW);
  digitalWrite(CE,HIGH);
  writeout(cmd_byte);
  reg_value=readin();
  digitalWrite(CE,LOW);
  return reg_value;
  }

void writeprotect(bool enable)
{
  writeregister(WP_REG, (enable<<7));
  }

void halt(bool enable)
{
  uint8_t sec = readregister(SEC_REG);
  sec &= ~(1<<7);
  sec|=(enable<<7);
  writeregister(SEC_REG, sec);
  }


void setupRTC()
{
  writeregister(0x80,0x01);
  writeregister(0x82,0x02);
  writeregister(0x84,0x03);
  writeregister(0x86,0x04);
  writeregister(0x88,0x05);
  writeregister(0x8A,0x06);
  writeregister(0x8C,0x07);
  }  
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(SCLK,OUTPUT);
  pinMode(DIO,OUTPUT);
  pinMode(CE,OUTPUT);
    
  writeprotect(false);
  halt(false);
  //setupRTC();
  //writeprotect(true);

  lcd.begin(16, 2);

}

void printtim()
{
  uint8_t sec,minu,hur,dat,mth,day,yer,h12,pm, ctmp,ctmp1;
  char disp_tmp[17];
  sec=readregister(SEC_REG);
  minu=readregister(MIN_REG);
  hur=readregister(HUR_REG);
  dat=readregister(DAT_REG);
  mth=readregister(MTH_REG);
  yer=readregister(YER_REG);
  day=readregister(DAY_REG);
  ctmp=(sec>>4)*10+((sec<<=4)>>4);
  sec=ctmp;
  ctmp=(minu>>4)*10+((minu<<4)>>4);
  minu=ctmp;
  h12=hur&0x80;
  if (h12)  ctmp=((hur<<3)>>7)*10+(hur<<4)>>4; else ctmp=((hur<<2)>>6)*10+(hur<<4)>>4;
  pm=hur&0x20;
  hur=ctmp;
  ctmp=(dat>>4)*10+((dat<<4)>>4);
  dat=ctmp;
  ctmp=(mth>>4)*10+((mth<<4)>>4);
  mth=ctmp;
  ctmp=(yer>>4)*10+((yer<<4)>>4);
  yer=ctmp;
  ctmp=(day<<4)>>4;
  lcd.clear();
  sprintf(disp_tmp,"%d-%d-%d D%d",yer,mth,dat,day);  
  lcd.setCursor(0,0);
  lcd.print(disp_tmp);
  sprintf(disp_tmp,"%d:%d:%d -%d%d",hur,minu,sec,h12,pm);
  lcd.setCursor(0,1);
  lcd.print(disp_tmp);
  }

void loop() {
  // put your main code here, to run repeatedly:
  printtim();
  delay(1000);
}
