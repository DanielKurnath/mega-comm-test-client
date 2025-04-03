#include <types.h>

bool Data_Available(void);

void Init_Serial(void);
void Allow_Incoming(void);
void Block_Incoming(void);
void Flush_Receive(void);

u8   Serial_Read(void);
void Serial_Write(u8 data);
void Serial_Write_Msg(char *str);


