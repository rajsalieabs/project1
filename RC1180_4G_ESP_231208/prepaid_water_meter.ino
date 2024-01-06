#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>
#include<EEPROM.h>

const int screen_width = 128;
const int screen_height = 64;

int count = 0;
const int sensor_pin = 2;
const int valve_pin = 8;
volatile int pulse_count = 0;

unsigned long int recharge_amount = 0 ;
unsigned long int total_pulse = 0;
unsigned long int total_liter = 0;

int eeprom_address = 0; 
int total_address = 4;   
int liter_address =8;

Adafruit_SSD1306 oled(screen_width,screen_height,&Wire,-1);

void setup()
{
  Serial.begin(9600);
  init_oled();
  init_pin();
  read_eeprom(); 
}
void init_pin()
{
  pinMode(sensor_pin,INPUT_PULLUP);
  pinMode(valve_pin,OUTPUT);
  pinMode(valve_pin,HIGH);
  attachInterrupt(digitalPinToInterrupt(sensor_pin),count_pulse,FALLING);
}
void init_oled()
{
  if(!oled.begin(SSD1306_SWITCHCAPVCC,0x3C))
  {
    Serial.println(F("SSD1306 FAILED"));
    while(true) ;
  }
  delay(2000);       
  oled.clearDisplay(); 
  oled.setTextSize(2);          
  oled.setTextColor(WHITE);    
  oled.setCursor(0, 10);        
  oled.println(" HELLO!!! "); 
  oled.display(); 
  delay(100);
}
void loop()
{
  read_serial_data();
  meter_evaluate();
  display_data();
  delay(100);
}

void count_pulse()
{
  pulse_count=1;
}

void meter_evaluate()
{
  if(digitalRead(sensor_pin) == HIGH)
  {
  if(recharge_amount > 0)
  {
     open_valve();
     open_oled_display();
    if(pulse_count == 1)
    {
      total_pulse = total_pulse+pulse_count;
      total_liter = total_pulse*10;
      recharge_amount= recharge_amount-10;
      pulse_count = 0;
      write_eeprom();
    }
  }
  else if(recharge_amount <= 0)
  {
    close_valve();
    close_oled_display();
    get_serial_input();
  }
  }
}
void open_valve()
{
  digitalWrite(valve_pin,LOW);
}

void close_valve()
{
  digitalWrite(valve_pin,HIGH);
}
void get_serial_input()
{
  Serial.println();
  Serial.print("U HAVE IN-SUFFICIENT BALANCE.\n");
  Serial.print("KINDLY RECHARGE.\n");
  Serial.println();
}

void display_data()
{
  Serial.print("BALANCE AMOUNT : ");
  Serial.print(recharge_amount);
  Serial.println();
  Serial.print("TOTAL_LITERS : ");
  Serial.print(total_liter);
  Serial.println('\n');
}

void open_oled_display()
{
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setCursor(1,10);
  oled.println("VALVE  ON");
  oled.display();
}

void close_oled_display()
{
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setCursor(1,10);
  oled.println("VALVE  OFF ");
  oled.display();
}

void write_eeprom()
{
  EEPROM.put(total_address,total_pulse);
  EEPROM.put(eeprom_address,recharge_amount);
  EEPROM.put(liter_address,total_liter);
}

void read_eeprom()
{
  EEPROM.get(total_address,total_pulse);
  EEPROM.get(eeprom_address,recharge_amount);
  EEPROM.get(liter_address,total_liter);
}

void read_serial_data()
{
  char buffer[32];
  long valve;
  int n = Serial.readBytesUntil('\n',buffer,sizeof(buffer)-1);
  buffer[n] = '\0';

  if(sscanf(buffer,"%*s %ld", &valve) == 1)
  {
    recharge_amount = recharge_amount+valve;
    write_eeprom();
  }
}
