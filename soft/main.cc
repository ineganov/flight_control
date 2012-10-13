//=================ADDRESS DEFINE'S====================//

#define DEFINE_EXTERNAL_DEVICE(NAME, ADDR) volatile int * const NAME = reinterpret_cast<volatile int * const>(ADDR);

DEFINE_EXTERNAL_DEVICE(DEV_JTAG,	0x00000000);

DEFINE_EXTERNAL_DEVICE(DEV_LEDS,	0x00000004);
DEFINE_EXTERNAL_DEVICE(DEV_ADC,		0x00000008);
DEFINE_EXTERNAL_DEVICE(DEV_IMU,		0x0000000C);
DEFINE_EXTERNAL_DEVICE(DEV_RTC,		0x00000010);

DEFINE_EXTERNAL_DEVICE(DEV_UART_TX,	0x00000014);
DEFINE_EXTERNAL_DEVICE(DEV_UART_RX,	0x00000018);

DEFINE_EXTERNAL_DEVICE(DEV_ENG_13,	0x0000001C);
DEFINE_EXTERNAL_DEVICE(DEV_ENG_24,	0x00000020);

DEFINE_EXTERNAL_DEVICE(DEV_UNUSED,	0x00000024);

DEFINE_EXTERNAL_DEVICE(DEV_RADIO_1,	0x00000028);
DEFINE_EXTERNAL_DEVICE(DEV_RADIO_2,	0x0000002C);
DEFINE_EXTERNAL_DEVICE(DEV_RADIO_3,	0x00000030);
DEFINE_EXTERNAL_DEVICE(DEV_RADIO_4,	0x00000034);
DEFINE_EXTERNAL_DEVICE(DEV_RADIO_5,	0x00000038);
DEFINE_EXTERNAL_DEVICE(DEV_RADIO_6,	0x0000003C);


//===============FUNCTION DECLARATIONS=================//

extern "C" void delay(int);
extern "C" int mymain();
static int  user_input();
static void leds(int);
static void usleep(int);
static int adc_read(int);

//===============FUNCTION DEFINITIONS==================//

static void usleep(int useconds)
{	delay(useconds*10); 
	return;	}

static int user_input()
{	return *DEV_LEDS; }

static void leds(int l)
{	*DEV_LEDS = l;	}

static int adc_read(int ch)
{   int r = -1;
    *DEV_ADC = ch;
    while (r < 0) r = *DEV_ADC;
    return r;	}

void print_hex(unsigned int a)
{	while(*DEV_UART_TX != 0) {usleep(10);}
	for(int i = 0; i < 8; ++i)
	{	unsigned char c = ((a >> 28) & 0xF);
		if (c < 10) c += '0';
		else		c += 'A' - 10;
		*DEV_UART_TX = c;
		a <<= 4;
	} 	
	*DEV_UART_TX = '\n'; }

void print_str(const char * str)
{	int free_space = 31 - *DEV_UART_TX;

	for(int i = 0; i < free_space; ++i)
	{	if(str[i]) *DEV_UART_TX = str[i];
		else return; }

	//we can choose to just wait 
	//usleep(1000000);
	//or wait for a specific condition, say if the buffer is half-full:
	while(*DEV_UART_TX > 15) {}

	print_str(str + free_space); } 

//===================MAIN==============================//
const unsigned char lv[] = {0x81, 0x42, 0x24, 0x18};
const char uart_msg[] = "Hello, a super-ultra-long string, which doesn't fit in the buffer!\n";

int main()
{	char buffer[64];
	
	for(int i = 0; i < 4; ++i)
	{	leds(lv[i]);
		usleep(300000); }	

	print_str("\nHello!\n");

	*DEV_ENG_13 = 0;
	*DEV_ENG_24 = 0;
	/*
	while(1)
	{	print_str("RADIO_CH1: "); print_hex(*DEV_RADIO_1);
		print_str("RADIO_CH2: "); print_hex(*DEV_RADIO_2);
		print_str("RADIO_CH3: "); print_hex(*DEV_RADIO_3);
		print_str("RADIO_CH4: "); print_hex(*DEV_RADIO_4);
		print_str("RADIO_CH5: "); print_hex(*DEV_RADIO_5);
		print_str("RADIO_CH6: "); print_hex(*DEV_RADIO_6);

		int throttle = *DEV_RADIO_1 - 1130;
		if(throttle < 0) throttle = 0; // --> 0..859 range
		if(*DEV_RADIO_1 < 0) throttle = 0;
		print_str("Throttle: "); print_hex(throttle);

		*DEV_ENG_24 = throttle ;

		print_str("------------------------\n");
		usleep(100000); }
	

	//throttle: 1128 - 1989

	*/

	int num_ch = 0;
	while(1)
	{	int char_count = 0xFFFF & (*DEV_UART_RX >> 16);
		for(int i = 0; i < char_count; i++)
		{	int ch = 0xFF & *DEV_UART_RX;
			*DEV_UART_RX = 0;
			
			//*DEV_UART_TX = ch; 
			buffer[num_ch] = ch;
			num_ch++;
			if(num_ch >= 64) 
			{	num_ch = 0;
				for(int j = 0; j < 64; j++) 
				{	*DEV_UART_TX = buffer[j];
					usleep(100000); }
				print_str("\n"); }
				
		}
		usleep(10000); }
	
	return 42; }

//=====================================================//
