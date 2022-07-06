/*
  Copyright (c) 2022 Engovis
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <Arduino.h>
#include <FlexCAN.h> //https://github.com/collin80/FlexCAN_Library

/////Version Identifier/////////
int firmver = 220706;

unsigned long looptime = 0;

uint16_t Chargers = 0;
uint8_t Chargerid = 0;
uint8_t tempid1 = 1;
uint8_t tempid2 = 1;

byte rxBuf[8];
char msgString[128];                        // Array to store serial string
uint32_t inbox;

CAN_message_t msg;
CAN_message_t inMsg;
CAN_filter_t filter;

//variables
int incomingByte = 0;
int x = 0;
int debug = 1;
int candebug = 1; //view can frames
int debugCur = 0;
int menuload = 0;


void setup() {
  // put your setup code here, to run once:

  Can0.begin(500000);

  Serial.begin(115200);
  Serial.println("Starting up!");
}

void loop()
{
  while (Can0.available())
  {
    canread();
  }

  if (Serial.available() > 0)
  {
    menu();
  }

  if (millis() - looptime > 500)
  {
    looptime = millis();
    if (debug == 1)
    {
      //sendCANChangeId(4, 6);
      Serial.println();
      Serial.println(millis());
      Serial.println("Chargers connected");
      if (Chargers > 0)
      {
        for (int y = 0; y < 16; y++)
        {


          if (Chargers & (0x01 << y))
          {
            Serial.print(" ID ");
            Serial.print(y + 1);
            Serial.print(" Found ");
            Serial.println();
          }
        }

      }
      else
      {
        Serial.print("No IDr Found ");
        Serial.println();
      }
      Serial.println();
      Serial.println("s - Menu");
      Serial.println();
      Serial.println();
      Serial.println();
    }
    sendCanUnlock(tempid1);
  }
}

void canread()
{
  Can0.read(inMsg);
  // Read data: len = data length, buf = data byte(s)
  if ((inMsg.id & 0x00F) == 0x008)
  {
    Chargerid = (inMsg.id & 0x0F0) >> 4;
    Chargers = Chargers | (0x01 << (Chargerid));
    /*
      Serial.print(Chargerid);
      Serial.print(" | ");
      Serial.print(Chargers);
      Serial.println();
    */
  }


  //if (candebug == 1 && (inMsg.id & 0x0F != 0x008))
  if ((inMsg.id & 0x00F) != 0x008)
  {
    Serial.print(millis());
    if ((inMsg.id & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (inMsg.id & 0x1FFFFFFF), inMsg.len);
    else
      sprintf(msgString, ",0x%.3lX,false,%1d", inMsg.id, inMsg.len);

    Serial.print(msgString);

    if ((inMsg.id & 0x40000000) == 0x40000000) {  // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for (byte i = 0; i < inMsg.len; i++) {
        sprintf(msgString, ", 0x%.2X", inMsg.buf[i]);
        Serial.print(msgString);
      }
    }
    Serial.println();
  }
}

void sendCanUnlock(int oldid)
{
  msg.id = (0x303 | ((oldid - 1) << 4));           // Set our transmission address ID
  msg.len = 8;            // Data payload 8 bytes
  msg.ext = 0;           // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 1;
  msg.buf[1] = 22;
  msg.buf[2] = 0xF1;
  msg.buf[3] = 0xE2;
  msg.buf[4] = 0xD3;
  msg.buf[5] = 0xC4;
  msg.buf[6] = 0xB5;
  msg.buf[7] = 0xA6;
  Can0.write(msg);
}

void sendCANChangeId(int oldid, int newid)
{
  msg.id = (0x303 | ((oldid - 1) << 4));           // Set our transmission address ID
  msg.len = 8;            // Data payload 8 bytes
  msg.ext = 0;           // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 1;
  msg.buf[1] = 22;
  msg.buf[2] = 0xF1;
  msg.buf[3] = 0xE2;
  msg.buf[4] = 0xD3;
  msg.buf[5] = 0xC4;
  msg.buf[6] = 0xB5;
  msg.buf[7] = 0xA6;
  Can0.write(msg);
  delay(3);

  msg.id = (0x303 | ( (oldid - 1) << 4));        // Set our transmission address ID
  msg.len = 3;            // Data payload 8 bytes
  msg.ext = 0;      // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 1;
  msg.buf[1] = 4;
  msg.buf[2] = newid;
  //        msg.buf[3]=0x00;
  //        msg.buf[4]=0x00;
  //        msg.buf[5]=0x00;
  //        msg.buf[6]=0x00;
  //        msg.buf[7]=0x00;
  Can0.write(msg);
}

void readver(int oldid)
{
  msg.id = (0x303 | ( (oldid - 1) << 4));        // Set our transmission address ID
  msg.len = 2;            // Data payload 8 bytes
  msg.ext = 0;      // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 0;
  msg.buf[1] = 12;
  //msg.buf[2] = 0x00;
  //        msg.buf[3]=0x00;
  //        msg.buf[4]=0x00;
  //        msg.buf[5]=0x00;
  //        msg.buf[6]=0x00;
  //        msg.buf[7]=0x00;
  Can0.write(msg);
}

void readproto(int oldid)
{
  msg.id = (0x303 | ( (oldid - 1) << 4));        // Set our transmission address ID
  msg.len = 2;            // Data payload 8 bytes
  msg.ext = 0;      // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 0x00;
  msg.buf[1] = 0x01;
  //msg.buf[2] = 0x00;
  //        msg.buf[3]=0x00;
  //        msg.buf[4]=0x00;
  //        msg.buf[5]=0x00;
  //        msg.buf[6]=0x00;
  //        msg.buf[7]=0x00;
  Can0.write(msg);
}

void writeproto(int oldid)
{
  msg.id = (0x303 | ((oldid - 1) << 4));           // Set our transmission address ID
  msg.len = 8;            // Data payload 8 bytes
  msg.ext = 0;           // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 1;
  msg.buf[1] = 22;
  msg.buf[2] = 0xF1;
  msg.buf[3] = 0xE2;
  msg.buf[4] = 0xD3;
  msg.buf[5] = 0xC4;
  msg.buf[6] = 0xB5;
  msg.buf[7] = 0xA6;
  Can0.write(msg);
  delay(3);

  msg.id = (0x303 | ( (oldid - 1) << 4));        // Set our transmission address ID
  msg.len = 2;            // Data payload 8 bytes
  msg.ext = 0;      // Extended addresses - 0=11-bit 1=29bit
  msg.buf[0] = 0x01;
  msg.buf[1] = 0x01;
  //msg.buf[2] = 0x00;
  //        msg.buf[3]=0x00;
  //        msg.buf[4]=0x00;
  //        msg.buf[5]=0x00;
  //        msg.buf[6]=0x00;
  //        msg.buf[7]=0x00;
  Can0.write(msg);
}

// Settings menu
void menu()
{
  incomingByte = Serial.read(); // read the incoming byte:

  if (menuload == 1)
  {
    switch (incomingByte)
    {
      case 'a':
        menuload = 1;
        if (Serial.available() > 0)
        {
          tempid1 = Serial.parseInt();
        }
        if (tempid1 > 16)
        {
          tempid1 = 1;
        }
        menuload = 0;
        incomingByte = 115;
        break;

      case 'b':
        menuload = 1;
        if (Serial.available() > 0)
        {
          tempid2 = Serial.parseInt();
        }
        if (tempid2 > 16)
        {
          tempid2 = 1;
        }
        menuload = 0;
        incomingByte = 115;
        break;

      case 'r':
        sendCANChangeId(tempid1, tempid2);
        Chargers = 0;
        break;

      case 'c':
        candebug = !candebug;
        menuload = 0;
        incomingByte = 115;
        break;

      case 'i':
        readver(tempid1);
        menuload = 0;
        incomingByte = 115;
        break;

      case 'h':
        readproto(tempid1);
        menuload = 0;
        incomingByte = 115;
        break;

      case 'j':
        writeproto(tempid1);
        menuload = 0;
        incomingByte = 115;
        break;

      case 'q': //q to go back to main menu
        menuload = 0;
        debug = 1;
        break;

      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }

  if (incomingByte == 115 & menuload == 0)
  {
    Serial.println();
    Serial.println("MENU Programming Eltek Ids");
    Serial.println("Debugging Paused");
    Serial.print("Firmware Version : ");
    Serial.println(firmver);
    Serial.println();

    Serial.print("a- Current Id: ");
    Serial.println(tempid1);
    Serial.print("b- New Id: ");
    Serial.println(tempid2);
    Serial.println();
    Serial.println("r - Reprogramme ID");
    Serial.println();
    Serial.println("i - Read Cur ID Software Ver");
    Serial.println();
    Serial.println("h - Read Protocol Ver");
    Serial.println();
    Serial.println("j - Write Eltek Protocl Ver");
    Serial.println();
    Serial.print("c - candebug : ");
    Serial.println(candebug);
    Serial.println();
    Serial.println("q - exit menu");
    debug = 0;
    menuload = 1;
  }
}
