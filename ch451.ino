
#define ch451_rst 0x0201
#define ch451_soff 0x0400
#define ch451_son 0x0403
#define ch451_dsp 0x0500
#define ch451_BCD 0x0580
#define ch451_blink 0x0600

#define ch451_DIG0 0x0800
#define ch451_DIG1 0x0900
#define ch451_DIG2 0x0A00
#define ch451_DIG3 0x0B00
#define ch451_DIG4 0x0C00
#define ch451_DIG5 0x0D00
#define ch451_DIG6 0x0E00
#define ch451_DIG7 0x0F00

#define ch451_DIN 13
#define ch451_Dclk 12
#define ch451_Load 11

int ch451_INT = 2;

int ch451_key = -1;

void onChange(){
  unsigned char i;
  unsigned char cmd, keycode;
  if (digitalRead(ch451_INT)==HIGH) return;
  cmd = 0x07;
  digitalWrite(ch451_Load,0);
  for(i=0; i<4; i++){
    digitalWrite(ch451_DIN,cmd&1);
    digitalWrite(ch451_Dclk,0);
    cmd>>=1;
    digitalWrite(ch451_Dclk,1);
    }
  digitalWrite(ch451_Load,1);
  keycode=0;
  for(i=0;i<7;i++){
    keycode<<=1;
    keycode|=digitalRead(ch451_INT);
    digitalWrite(ch451_Dclk,0);
    digitalWrite(ch451_Dclk,1);
    }
    pinMode(ch451_INT,OUTPUT);
    digitalWrite(ch451_INT,1);
    pinMode(ch451_INT,INPUT);
    ch451_key=keycode;
  }


const unsigned char DatCode[18]={ 0x3F,0x06,0x5B,0x4F, //0123
    0x66,0x6D,0x7D, //4567
    0x7F,0x6F,0x77,0x7C, //89AB
    0x39,0x5E,0x79,0x71, //CdEF
    0x40,0x00
  };

const unsigned int DigCode[9]={ch451_DIG0, ch451_DIG1, ch451_DIG2, ch451_DIG3, ch451_DIG4,
ch451_DIG5, ch451_DIG6, ch451_DIG7};

const unsigned int table[]={0x0000,0x0001,0x0002,0x0003,0x0004,
0x0005,0x0006,0x0007,0x0008,0x0009};

void write_ch451(unsigned int cmd){
  unsigned char i;
  digitalWrite(ch451_Load,0);
  for(i=0;i<12;i++){
    digitalWrite(ch451_DIN,cmd&1);
    digitalWrite(ch451_Dclk,0);
    cmd=cmd>>1;
    digitalWrite(ch451_Dclk,1);
    }
    digitalWrite(ch451_Load,1);
  }

void ch451_init(){
  digitalWrite(ch451_DIN,0);
  digitalWrite(ch451_DIN,1);
  write_ch451(ch451_rst);
  write_ch451(ch451_son);
  write_ch451(ch451_BCD);
  write_ch451(ch451_blink);
  }

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(ch451_INT), onChange, CHANGE);   //setup interrupt
  
  Serial.begin(9600);
  pinMode(ch451_DIN,OUTPUT);
  pinMode(ch451_Dclk,OUTPUT);
  pinMode(ch451_Load,OUTPUT);
  
  ch451_init();
  //prnt();
}

void prnt(){
  write_ch451(0x0800);
  write_ch451(0x0901);
  write_ch451(0x0a02);
  write_ch451(0x0b03);
  write_ch451(0x0c04);
  write_ch451(0x0d85);
  write_ch451(0x0e06);
  write_ch451(0x0f07);  
}

int getkey(){
  ch451_key=-1;
  //Serial.write("geting..");
  while (ch451_key<0){
    
    Serial.write(ch451_key);
    //delay(100);
    }
  //Serial.write("got");  
  return ch451_key;
  }

int cmd0;
unsigned char ch,ch1;

void loop() {
  cmd0=getkey();
 Serial.write(cmd0);
    ch=cmd0&0xff;
    ch1=ch>>4;
    write_ch451(0x0800|ch1);
    ch1=ch<<4;
    ch1>>=4;
    write_ch451(0x0900|ch1);
 //delay(500);
  //Serial.write(getkey());
}
