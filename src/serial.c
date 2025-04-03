#include <genesis.h>
#include <serial.h>

#define PORT2_CTRL  0xA1000B
#define PORT2_DATA  0xA10005
#define PORT2_SCTRL 0xA10019
#define PORT2_TX    0xA10015
#define PORT2_RX    0xA10017

#define SCTRL_TFUL  0x1
#define SCTRL_RRDY  0x2
#define SCTRL_RERR  0x3

// ************************************************************
// SERIAL SEND/RECEIVE ROUTINES
// ************************************************************

// PINS 1,2,3,6 ARE OUTPUTS. REST ARE INPUTS. 4800 BAUD
void Init_Serial(void) 
{ 
  asm("MOVE.B #0x57,0xA1000B\t\n"); // 1,2,3,6 PINS ARE OUTPUTS, REST ARE INPUTS 01010111
  asm("MOVE.B #0x30,0xA10019\t\n"); // Init controller port 2 for 4800 baud serial
}

bool Data_Available(void)
{
  vu8* pb = (u8*)PORT2_SCTRL;               // Get serial status into pb
  return *pb & SCTRL_RRDY;                  // return it
}

u8 Serial_Read(void) 
{ 
  vu8* pb = (u8*)PORT2_RX; 
  return *pb;
} 

void Serial_Write(u8 data) 
{
  volatile uint8_t *pb;
  pb = (volatile uint8_t *) PORT2_SCTRL;
  while((*pb & SCTRL_TFUL) != 0x00) { pb = (volatile uint8_t *) PORT2_SCTRL; } // wait for TFUL (Bit 0) to become 0 (ok to send)
  pb = (volatile uint8_t *) PORT2_TX; // pb = PORT2_TX
  *pb = data; // pointer pb = data value
}

void Serial_Write_Msg(char *str)
{
  int i=0,len=0;
  len=strlen(str);
  char data[len+1];
  strcpy(data,str);
  while ( i < len) { Serial_Write(data[i]); i++; }  
}

//****************************************************************
// Enter Monitor Mode                                           **
//****************************************************************
void NET_enterMonitorMode(void)
{
    Serial_Write_Msg("C0.0.0.0/0\n");
    while (!Data_Available() || Serial_Read() != '>') {}
}

//****************************************************************
// Exit Monitor Mode                                            **
//****************************************************************
void NET_exitMonitorMode(void)
{
    Serial_Write_Msg("QU\n");
    while (!Data_Available() || Serial_Read() != '>') {}
}
