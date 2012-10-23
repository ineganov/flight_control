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

const unsigned int MPU6050_ADDR = 0x1D0;

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

static void i2c_start()
{   *DEV_IMU = 0x000400FF;
    while (*DEV_IMU < 0);	}

static void i2c_stop() {
    *DEV_IMU = 0x000100FF;
    while (*DEV_IMU < 0);	}

static int i2c_io(int b) {
    b &= 0x1FF;
    *DEV_IMU = 0x00020000 | b;
    while (*DEV_IMU < 0);
    return *DEV_IMU;	}

void mpu6050_write(int reg, int byte)
{   i2c_start();
    i2c_io(MPU6050_ADDR);
    i2c_io(0x100 | reg);
    i2c_io(0x100 | byte);
    i2c_stop();	}

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

	mpu6050_write(0x6B, 0x80); //Device reset
	delay(100000);			   //approx 5ms grace period
	mpu6050_write(0x6B, 0x00); //Sleep disable

	mpu6050_write(0x1A, 0x03); //Low-pass ON
	mpu6050_write(0x1B, 0x18); //GYRO_CONFIG: +-2000 dps range
	mpu6050_write(0x1C, 0x18); //ACC_CONFIG: +-16g range

	int a, b, c, d, gX, gY; 
	int ax, axL, ay, ayL, az, azL;
	int gx, gxL, gy, gyL, gz, gzL;

	while(1)
	{	i2c_start();
		i2c_io(0x1D0);	//MPU6050 ADDR
		i2c_io(0x13B);  //Accel
		i2c_stop();
		i2c_start();

		i2c_io(0x1D1);	
		ax  = 0xFF & i2c_io(0x0FF);	
		axL = 0xFF & i2c_io(0x0FF);
		ay  = 0xFF & i2c_io(0x0FF);	
		ayL = 0xFF & i2c_io(0x0FF);
		az  = 0xFF & i2c_io(0x0FF);	
		azL = 0xFF & i2c_io(0x0FF);
		
		i2c_io(0x0FF);	
		i2c_io(0x0FF);

		gx  = 0xFF & i2c_io(0x0FF);	
		gxL = 0xFF & i2c_io(0x0FF);
		gy  = 0xFF & i2c_io(0x0FF);	
		gyL = 0xFF & i2c_io(0x0FF);
		gz  = 0xFF & i2c_io(0x0FF);	
		gzL = 0xFF & i2c_io(0x1FF);
		i2c_stop();	

		print_str("ax: "); print_hex(ax);
		print_str("axL: "); print_hex(axL);

		usleep(1000000);	}


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
	*/

	//throttle: 1128 - 1989

	

	int num_ch = 0;
	while(1)
	{	int char_count = 0xFFFF & (*DEV_UART_RX >> 16);
		for(int i = 0; i < char_count; i++)
		{	int ch = 0xFF & *DEV_UART_RX;
			*DEV_UART_RX = 0;
			
			//*DEV_UART_TX = ch; 
			buffer[num_ch] = ch;
			num_ch++;
			if(num_ch >= 32) 
			{	num_ch = 0;
				for(int j = 0; j < 32; j++) 
				{	*DEV_UART_TX = buffer[j];
					/*usleep(100000);*/ }
				print_str("\n"); }
				
		}
		usleep(10000); }
	
	return 42; }

//=====================================================//
