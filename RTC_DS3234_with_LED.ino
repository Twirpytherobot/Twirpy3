/* 
 *  RTC_DS3234
 *  
 *  Pin Connections for Arduino UNO
 *  
 *  RTC         UNO
 *  0V          GND
 *  +V          5V
 *  CLK         13
 *  DO          12
 *  DI          11
 *  CS          8
 */
 
#include "LedControl.h"
#include <SPI.h>
//button
const int button = 3;
int buttonState = 0;
//clock
const int  cs=8; //chip select 

int disTime [7]; ////second,minute,hour,null,day,month,year  numbers to mimic the read time digits
//leds
int incomingByte = 0;
unsigned long delayTime=200;  // Delay between Frames

const uint64_t DIGITS[] = {
  0x3c66666e76663c00, //0 
  0x7e1818181c181800, //1
  0x7e060c3060663c00, //2
  0x3c66603860663c00, //3
  0x30307e3234383000, //4
  0x3c6660603e067e00, //5
  0x3c66663e06663c00, //6
  0x1818183030667e00, //7
  0x3c66663c66663c00, //8
  0x3c66607c66663c00, //9
  0x6ff6969697f66600, //10
  0xff66666677666600, // 11
  0xff3666c6dff66600, //12
  0x0018180000181800, //Colon 13
  0x0000000000000000 //space 14

};
const int DIGITS_LEN = sizeof(DIGITS)/8;


LedControl lc=LedControl(6,7,5,1);  // Pins: DIN,CLK,CS, # of Display connected


void setup() {
  pinMode(button, INPUT);
  lc.clearDisplay(0);  // Clear Displays
  lc.shutdown(0,false);  // Wake up displays
  lc.setIntensity(0,3);  // Set intensity levels
  Serial.begin(9600);
  RTC_init();
  //day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)
  //SetTimeDate(27,7,18,22,11,00); 
}

void displayImage(uint64_t image) {
  for(int i = 0; i< 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j<8; j++) {
      lc.setLed(0,i,j,bitRead(row,j));
    }
  }
}

int i = 0;
void RTC_init(){ 
    pinMode(cs,OUTPUT); // chip select
    // start the SPI library:
    SPI.begin();
    SPI.setBitOrder(MSBFIRST); 
    SPI.setDataMode(SPI_MODE3); 
    //set control register 
    digitalWrite(cs, LOW);  
    SPI.transfer(0x8E);
    SPI.transfer(0x60); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
    digitalWrite(cs, HIGH);
    delay(10);
}
//sets the time
void SetTimeDate(int d, int mo, int y, int h, int mi, int s){ 
  int TimeDate [7]={s,mi,h,0,d,mo,y};
 int disTime [7] = {s,mi,h,0,d,mo,y};
  for(int i=0; i<=6;i++){
    if(i==3)
      i++;
    int b= TimeDate[i]/10;
    int a= TimeDate[i]-b*10;
    if(i==2){
      if (b==2)
        b=B00000010;
      else if (b==1)
        b=B00000001;
    } 
    TimeDate[i]= a+(b<<4);
      
    digitalWrite(cs, LOW);
    SPI.transfer(i+0x80); 
    SPI.transfer(TimeDate[i]);        
    digitalWrite(cs, HIGH);
  }
}
//puts together time string
String ReadTimeDate(){
  String temp;
  int TimeDate [7]; //second,minute,hour,null,day,month,year    
  for(int i=0; i<=6;i++){
    if(i==3)
      i++;
    digitalWrite(cs, LOW);
    SPI.transfer(i+0x00); 
    unsigned int n = SPI.transfer(0x00);        
    digitalWrite(cs, HIGH);
    int a=n & B00001111;    
    if(i==2){ 
      int b=(n & B00110000)>>4; //24 hour mode
      if(b==B00000010)
        b=20;        
      else if(b==B00000001)
        b=10;
      TimeDate[i]=a+b;
      disTime[i]=TimeDate[i];
    }
    else if(i==4){
      int b=(n & B00110000)>>4;
      TimeDate[i]=a+b*10;
      disTime[i]=TimeDate[i];
    }
    else if(i==5){
      int b=(n & B00010000)>>4;
      TimeDate[i]=a+b*10;
      disTime[i]=TimeDate[i];
    }
    else if(i==6){
      int b=(n & B11110000)>>4;
      TimeDate[i]=a+b*10;
      disTime[i]=TimeDate[i];
    }
    else{ 
      int b=(n & B01110000)>>4;
      TimeDate[i]=a+b*10; 
      disTime[i]=TimeDate[i];
      }
  }
  temp.concat(TimeDate[4]);
  temp.concat("/") ;
  temp.concat(TimeDate[5]);
  temp.concat("/") ;
  temp.concat(TimeDate[6]);
  temp.concat(" ") ;
  temp.concat(TimeDate[2]);
  temp.concat(":") ;
  temp.concat(TimeDate[1]);
  temp.concat(":") ;
  temp.concat(TimeDate[0]);
  return(temp);
}
//Display function
void showTime(){
  buttonState = digitalRead(button);
  if (buttonState == HIGH){
    //day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)
    SetTimeDate(28,7,18,21,38,00); 
  }
  displayImage(DIGITS[((disTime[2] % 100)/10)]);
  delay(500);
  displayImage(DIGITS[14]);
  delay(250);
  displayImage(DIGITS[(disTime[2] % 10)]);
  delay(500);
  displayImage(DIGITS[13]);
  delay(250);
  displayImage(DIGITS[14]);
  delay(250);
  displayImage(DIGITS[((disTime[1] % 100)/10)]);
  delay(500);
  displayImage(DIGITS[14]);
  delay(250);
  displayImage(DIGITS[(disTime[1] % 10)]);
  delay(500);
}
//Main loop
void loop() {
  showTime();
  Serial.println(ReadTimeDate());
  Serial.println(disTime[1] % 10);
  Serial.println((disTime[1] % 100) /10);
  Serial.println(disTime[1]);
  Serial.println(buttonState);
  delay(500);
}


