/*
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 8
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
*/
#include <LiquidCrystal.h>
#include "dht.h"

dht DHT;

#define DHT11_PIN 9 //put the sensor in the digital pin 13

#define SCLK 7
#define DIO 6
#define CE 5

#define SEC_REG 0
#define MIN_REG 1
#define HUR_REG 2
#define DAT_REG 3
#define MTH_REG 4
#define YER_REG 6
#define DAY_REG 5
#define WP_REG 7

typedef uint8_t reg_t;
//  LCD config
const int rs = 12, en = 11, d4 = 8, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//
char disp_tmp[17];

void writeout(uint8_t value)
{
  pinMode(DIO, OUTPUT);
  shiftOut(DIO, SCLK, LSBFIRST, value);
}

uint8_t readin()
{
  uint8_t input_value = 0;
  uint8_t bit = 0;
  pinMode(DIO, INPUT);
  for (int i = 0; i < 8; i++)
  {
    bit = digitalRead(DIO);
    input_value |= (bit << i);
    digitalWrite(SCLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SCLK, LOW);
  }
  return input_value;
}

void writeregister(reg_t reg, uint8_t value)
{
  uint8_t cmd_byte = (128 | (reg << 1));
  digitalWrite(SCLK, LOW);
  digitalWrite(CE, HIGH);
  writeout(cmd_byte);
  writeout(value);
  digitalWrite(CE, LOW);
}

uint8_t readregister(reg_t reg)
{
  uint8_t cmd_byte = 0x81;
  uint8_t reg_value;
  cmd_byte |= (reg << 1);
  digitalWrite(SCLK, LOW);
  digitalWrite(CE, HIGH);
  writeout(cmd_byte);
  reg_value = readin();
  digitalWrite(CE, LOW);
  return reg_value;
}

void writeprotect(bool enable)
{
  writeregister(WP_REG, (enable << 7));
}

void halt(bool enable)
{
  uint8_t sec = readregister(SEC_REG);
  sec &= ~(1 << 7);
  sec |= (enable << 7);
  writeregister(SEC_REG, sec);
}

int readdht11() 
{
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
  case 0:
    Serial.print("OK,\t");
    break;
  case -1:
    Serial.print("Checksum error,\t");
    break;
  case -2:
    Serial.print("Time out error,\t");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }
  // DISPLAT DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);
  return chk;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(SCLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  pinMode(CE, OUTPUT);

  writeprotect(false);
  halt(false);
  //setupRTC();
  writeprotect(true);

  lcd.begin(16, 2);
  lcd.clear();
  int idht=readdht11();
  sprintf(disp_tmp,"DHT Ret:%d",idht);
  lcd.print(disp_tmp);
  delay(1000);
}

void printtim()
{
  uint8_t sec, minu, hur, dat, mth, day, yer, h12, pm, ctmp, ctmp1;
  sec = readregister(SEC_REG);
  minu = readregister(MIN_REG);
  hur = readregister(HUR_REG);
  dat = readregister(DAT_REG);
  mth = readregister(MTH_REG);
  yer = readregister(YER_REG);
  day = readregister(DAY_REG);
  ctmp = (sec >> 4) * 10 + (sec & 0x0f);
  sec = ctmp;
  ctmp = (minu >> 4) * 10 + (minu & 0x0f);
  minu = ctmp;
  h12 = hur & 0x80;
  if (h12)
    ctmp = ((hur << 3) >> 7) * 10 + (hur & 0x0f);
  else
    ctmp = ((hur << 2) >> 6) * 10 + (hur & 0x0f);
  pm = hur & 0x20;
  hur = ctmp;
  ctmp = (dat >> 4) * 10 + (dat & 0x0f);
  dat = ctmp;
  ctmp = (mth >> 4) * 10 + (mth & 0x0f);
  mth = ctmp;

  ctmp1 = (yer >> 4) * 10;
  ctmp = yer & 0x0f;
  yer = ctmp1 + ctmp;

  lcd.clear();
  sprintf(disp_tmp, "%02d-%02d %02d:%02d:%02d %01d", mth, dat, hur, minu, sec, day - 1);
  lcd.setCursor(0, 0);
  lcd.print(disp_tmp);
  bool idht=readdht11()==0;
  if (idht){
    sprintf(disp_tmp,"%3dc %3d%%",(int)DHT.temperature,(int)DHT.humidity);
    lcd.setCursor(0,1);
    lcd.print(disp_tmp);
    }
}

void loop()
{
  // put your main code here, to run repeatedly:
  printtim();
  delay(1000);
}
