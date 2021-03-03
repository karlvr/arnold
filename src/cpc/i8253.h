typedef struct
{
	int Counters[3];
	int TimeConstant[3];
	int ControlRegister[3];
	
} i8253;

void i8253_reset(i8253 *timer);
void i8253_write(i8253 *timer, int Register, int Data);
