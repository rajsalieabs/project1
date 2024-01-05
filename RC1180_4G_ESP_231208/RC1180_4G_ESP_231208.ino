/*
 * hello this is a code to read WMBUS frames from Axioma meter using RC1180 Module
 * Written by Raj Vignesh
 * */
#include <SoftwareSerial.h>

#define mbus Serial1

#define PWR_Key  33
#define En_Modem  32

#define CHANNEL 'C'
#define CFIELD 'F'
#define MBUS_MODE 'G'
#define INSTALL 'I'

#define MEMORY_CONFIG 'M'
#define QUALITY_INDICATOR 'Q'
#define READ_MAILBOX 'R'
#define RSSI_VAL 'S'
#define TEMPERATURE 'U'
#define VOLTAGE 'V'

#define REGISTER_MODE 0x03
#define REGISTER_ROLE 0x12
#define REGISTER_BAUD_RATE 0x30

#define MODE_S1 0x00
#define MODE_T1 0x01
#define MODE_T2 0x02
#define MODE_S2 0x03
#define MODE_R 0x04
#define MODE_T1C 0x0A
#define MODE_T2C 0x0B

#define INSTALL_NORMAL 0x00
#define INSTALL_INSTALL_MODE 0x01
#define INSTALL_ACCEPT_ALL 0x02

#define ROLE_SLAVE 0x00
#define ROLE_MASTER 0x01
#define ROLE_REPEATER 0x02

#define CFIELD_SND_NR 0x44
#define CFIELD_SND_UD 0x43

#define BAUD_19200 0x05
#define BAUD_57600 0x08
#define BAUD_115200 0x0A

String manId, ver, type;
String data = "Dilogy_1008\"";

byte mBusData [100];

int bufferLength = sizeof(mBusData);
boolean modeSettingFlag = 0;

//Decoded outcomes to be stored here
byte frameLength;
uint16_t mfrID;
String meterID;
unsigned int txnCount;
unsigned long totaliser;
byte battery;
boolean telegramDecodedFlag = 0;


void setup()
{
  Serial.begin(115200);
  pinMode(PWR_Key, OUTPUT);
  pinMode(En_Modem, OUTPUT);
  GSM_MbusEnableFn();
  mbus.begin(19200, SERIAL_8N1, 26, 27);
  delay(1000);

  factoryReset("Factory Resetting");
  resetRadio("Reset Radio");
  configNVMem(REGISTER_ROLE, ROLE_MASTER, "Set NW role to Master");
  configVolMem(MBUS_MODE, MODE_T1, "Set mode to T1");        //Set Mode to T1
  getFrmModule(RSSI_VAL, "Get RSSI Value");
}

void loop()
{
  Serial.println("Starting Acquisition");
  wmbus();
  Serial.println("End of Acquisition");
  if(telegramDecodedFlag == 1)
  {
    //Transmit over LTE code to come here
    telegramDecodedFlag = 0;
  }
  Serial.println();
  delay(2000);
}

void GSM_MbusEnableFn()
{
  Serial.print("GSM Power For Mbus and 4G...");
  digitalWrite(En_Modem, HIGH);
  delay(2000);
  digitalWrite(En_Modem, LOW);
  delay(2000);
  digitalWrite(PWR_Key, LOW);
  delay(2000);
  digitalWrite(PWR_Key, HIGH);
  delay(10000);
  Serial.println("  On Done");
}

void wmbus()
{
  configNVMem(REGISTER_ROLE, ROLE_MASTER, "Set NW role to Master");
  configVolMem(MBUS_MODE, MODE_T1, "Set mode to T1");        //Set Mode to T1
  memset(mBusData, 0, bufferLength);
  bufferLength = 0;
  unsigned long then = millis() / 1000;
  while ((millis() / 1000) - then <= 10 && bufferLength < 200) //For 5 seconds or maximum 200 bytes.
  {
    if (mbus.available())
    {
      mBusData[bufferLength] = mbus.read();
      Serial.print(mBusData[bufferLength], HEX);
      bufferLength++;
      Serial.print(",");
    }
  }
  Serial.println();
  //mbus.flush();
  //mbus.end();
}

void configNVMem(byte reg, byte val, String comment)
{
  Serial.println("---SoC---");
  Serial.println(comment);
  sendCmdHex(0x00);
  sendCmdChar('M');
  sendCmdHex(reg);
  sendCmdHex(val);
  sendCmdHex(0xFF);
  sendCmdChar('X');
  Serial.println("---EoC---\n");
}

void configVolMem(byte reg, byte val, String comment)
{
  Serial.println("---SoC---");
  Serial.println(comment);
  sendCmdHex(0x00);
  sendCmdChar(reg);
  sendCmdHex(val);
  sendCmdChar('X');
  Serial.println("---EoC---\n");
}

void readMailbox(byte mailBoxNo, String comment)
{
  configVolMem(READ_MAILBOX, mailBoxNo, comment);
}

void sendCmdChar(char cmd)
{
  Serial.print("Sending: '");
  Serial.print(cmd);
  Serial.print("'");
  mbus.write(cmd);
  delay(100);
  Serial.print("\tReceived: ");
  if (mbus.available())
  {
    Serial.print(mbus.read());
    Serial.print(' ');
  }
  Serial.println();
}

void sendCmdStr(String cmd)
{
  Serial.print("Sending: ");
  Serial.print(cmd);
  mbus.print(cmd);
  delay(100);
  Serial.print("\tReceived: ");
  if (mbus.available())
  {
    Serial.print(mbus.read());
    Serial.print(' ');
  }
  Serial.println();
}

void sendCmdHex(byte cmd)
{
  Serial.print("Sending: 0x");
  Serial.print(cmd, HEX);
  mbus.write(cmd);
  delay(100);
  Serial.print("\tReceived: ");
  if (mbus.available())
  {
    Serial.print(mbus.read());
    Serial.print(' ');
  }
  Serial.println();
}

void factoryReset(String comment)
{
  Serial.println(comment);
  sendCmdHex(0x00);
  sendCmdStr("@RC");
  sendCmdChar('X');
}

void resetRadio(String comment)
{
  Serial.println(comment);
  sendCmdHex(0x00);
  sendCmdStr("@RR");
  sendCmdChar('X');
}

void getFrmModule(char prefix, String comment)
{
  Serial.println("---SoG---");
  Serial.println(comment);
  sendCmdHex(0x00);
  sendCmdChar(prefix);
  sendCmdChar('X');
  Serial.println("---EoG---\n");
}
