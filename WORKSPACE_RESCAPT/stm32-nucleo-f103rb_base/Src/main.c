#include "main.h"
//###################################################################
#define VL6180X	0
#define MPU9250	1
#define MPL115A_ANEMO 0
//###################################################################

//====================================================================
//			CAN ACCEPTANCE FILTER
//====================================================================
#define USE_FILTER	1
#define M_PI  3.14159265358979323846
// Can accept until 4 Standard IDs
#define ID_IHM	                0xA0 	// IHM
#define ID_ANEMO_PRESSURE_CARD	0xC1	// pression / anemo sensor
#define ID_LUX_RANGE_CARD	    0xC2	// lux / distance sensor
#define ID_IMU_CARD        	    0xC3	// gyroscope sensor
//====================================================================
extern void systemClock_Config(void);

void (*rxCompleteCallback) (void);
void can_callback(void);

CAN_Message      rxMsg;
CAN_Message      txMsg;
long int        counter = 0;

uint8_t* aTxBuffer[2];

extern float magCalibration[3];

void VL6180x_Init(void);
void VL6180x_Step(void);

int status;
int new_switch_state;
int switch_state = -1;

float pressure;
float temperature;
float anemo_speed;

float phi;
float teta;
float psi;
//====================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>>> MAIN <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//====================================================================
int main(void)
{
	HAL_Init();
	systemClock_Config();
    SysTick_Config(HAL_RCC_GetHCLKFreq() / 1000); //SysTick end of count event each 1ms
	uart2_Init();


#if VL6180X || MPU9250
	i2c1_Init();
#endif

	HAL_Delay(1000); // Wait

#if MPL115A_ANEMO
    spi1_Init();
    anemo_Timer1Init();
#endif

#if VL6180X
    VL6180x_Init();
#endif

#if MPU9250
    mpu9250_InitMPU9250();
    mpu9250_CalibrateMPU9250();
#if USE_MAGNETOMETER
    mpu9250_InitAK8963(magCalibration);
#endif
    uint8_t response=0;
	response =  mpu9250_WhoAmI();
	//term_printf("%d",response);
#endif

    can_Init();
    can_SetFreq(CAN_BAUDRATE); // CAN BAUDRATE : 500 MHz -- cf Inc/config.h
#if USE_FILTER
    #if MPL115A_ANEMO
        can_Filter_list((ID_ANEMO_PRESSURE_CARD<<21)|(ID_ANEMO_PRESSURE_CARD<<5) , (ID_ANEMO_PRESSURE_CARD<<21)|(ID_ANEMO_PRESSURE_CARD<<5) , CANStandard, 0); // Accept until 4 Standard IDs
    #elif VL6180X
        can_Filter_list((ID_LUX_RANGE_CARD<<21)|(ID_LUX_RANGE_CARD<<5) , (ID_LUX_RANGE_CARD<<21)|(ID_LUX_RANGE_CARD<<5) , CANStandard, 0); // Accept until 4 Standard IDs
    #elif MPU9250
        can_Filter_list((ID_IMU_CARD<<21)|(ID_IMU_CARD<<5) , (ID_IMU_CARD<<21)|(ID_IMU_CARD<<5) , CANStandard, 0); // Accept until 4 Standard IDs
    #else
        can_Filter_list((ID_IHM<<21)|(ID_ANEMO_PRESSURE_CARD<<5) , (ID_LUX_RANGE_CARD<<21)|(ID_IMU_CARD<<5) , CANStandard, 0); // Accept until 4 Standard IDs
    #endif
#else
    can_Filter_disable(); // Accept everybody
#endif
    can_IrqInit();
    can_IrqSet(&can_callback);

    /*txMsg.id=0x55;
    txMsg.data[0]=1;
    txMsg.data[1]=2;
    txMsg.len=2;
    txMsg.format=CANStandard;
    txMsg.type=CANData;

    can_Write(txMsg);

    float i = -0.045;
    term_printf("%f",i);*/

    // D??commenter pour utiliser ce Timer ; permet de d??clencher une interruption toutes les N ms
    // Le programme d'interruption est dans tickTimer.c
    tickTimer_Init(1000); // period in ms

    while (1) {

#if VL6180X
    VL6180x_Step();

#endif

#if MPU9250

    get_acceleroscope_gyroscope();
    HAL_Delay(2);

    //send_axes();
#endif

#if MPL115A_ANEMO
    SPI1_Mesure m;
    m = spi1_get_pressure_temperature();
    pressure = m.pressure;
    temperature = m.temperature;
    anemo_speed = anemo_GetSpeed(1);

    /*send_pressure();
    HAL_Delay(50);
    send_temperature();
    HAL_Delay(50);
    send_wind_speed();
    HAL_Delay(50);*/
#endif
    }
	return 0;
}

void get_acceleroscope_gyroscope() {
	// get accelerometer and gyroscope
	int16_t data[6];
	mpu9250_Step(data);

	float ax = data[0]*(4.0/32768.0);
	float ay = data[1]*(4.0/32768.0);
	float az = data[2]*(4.0/32768.0);

	float gx = data[3]*(1000.0/32768.0)*(M_PI/180.0);
	float gy = data[4]*(1000.0/32768.0)*(M_PI/180.0);
	float gz = data[5]*(1000.0/32768.0)*(M_PI/180.0);

	// correction Euler
	madgwickAHRSupdateIMU(gx,gy,gz,ax,ay,az);

	float R11 = q0*q0 + q1*q1 - q2*q2 - q3*q3;
	float R12 = 2*(q1*q2 - q0* q3);
	float R13 = 2*(q1*q3 + q0*q2);

	float R21 = 2*(q0*q3 + q1*q2);
	float R22 = q0*q0 - q1*q1 + q2*q2 -q3*q3;
	float R23 = 2*(q2*q3 - q0*q1);

	float R31 = 2*(q1*q3 - q0*q2);
	float R32 = 2*(q2*q3 + q0*q1);
	float R33 = q0*q0 - q1*q1 - q2*q2 + q3*q3;

	// Calcul des angle d'Euler a partir de la matrice de rotation(angle en degr??)
	phi 	= atan2(-R31, R33)*(180.0/M_PI)+180.0;
	teta 	= (asin(R32)*(180.0/M_PI))+180.0;
	psi 	= atan2(-R12,R22)*(180.0/M_PI)+180.0;
}



////////////////////////////////
// SEND FUNCTIONS

void send_can(int from_id, int to_id, char data_type, unsigned char *data, int len)
{
	txMsg.id = to_id;
	txMsg.len=len+2;
	txMsg.data[0] = from_id;
	txMsg.data[1] = (unsigned char) data_type;

	for (int i=0; i<len; i++) {
		txMsg.data[i+2] = data[i];
	}

	txMsg.format = CANStandard;
	txMsg.type	 = CANData;
	can_Write(txMsg);
}

void send_temperature(void)
{
	unsigned char data[] = {
		(unsigned char) temperature>>24,
		(unsigned char) temperature>>16,
		(unsigned char) temperature>>8,
		(unsigned char) temperature & 0x000000FF
	};
	send_can(ID_ANEMO_PRESSURE_CARD, ID_IHM, 'T', data, (int) sizeof(data) / sizeof(data[0]));
	term_printf("Temperature: %f ??C\n", temperature);
}

void send_pressure(void)
{
	unsigned char data[] = {
		(unsigned char) pressure>>24,
		(unsigned char) pressure>>16,
		(unsigned char) pressure>>8,
		(unsigned char) pressure & 0x000000FF
	};
	send_can(ID_ANEMO_PRESSURE_CARD, ID_IHM, 'P', data, (int) sizeof(data) / sizeof(data[0]));
	term_printf("Pressure: %f kPa\n", pressure);
}

void send_wind_speed(void)
{
	unsigned char data[] = {
		(unsigned char) anemo_speed>>24,
		(unsigned char) anemo_speed>>16,
		(unsigned char) anemo_speed>>8,
		(unsigned char) anemo_speed & 0x000000FF
	};
	send_can(ID_ANEMO_PRESSURE_CARD, ID_IHM, 'W', data, (int) sizeof(data) / sizeof(data[0]));
	term_printf("Wind Speed: %f km/h\n", anemo_speed);
}

void send_pressure_temperature_wind_speed(void) {
	unsigned char data[] = {
		(unsigned char) pressure>>8,
		(unsigned char) pressure & 0x000000FF,
		(unsigned char) temperature>>8,
		(unsigned char) temperature & 0x000000FF,
		(unsigned char) anemo_speed>>8,
		(unsigned char) anemo_speed & 0x000000FF
	};
	send_can(ID_ANEMO_PRESSURE_CARD, ID_IHM, 'B', data, (int) sizeof(data) / sizeof(data[0]));
}

void send_distance(void)
{
	unsigned char data[] = {
		(unsigned char) Range.range_mm>>24,
		(unsigned char) Range.range_mm>>16,
		(unsigned char) Range.range_mm>>8,
		(unsigned char) Range.range_mm & 0x000000FF
	};
	send_can(ID_LUX_RANGE_CARD, ID_IHM, 'R', data, (int) sizeof(data) / sizeof(data[0]));
	term_printf("distance: %d mm\n", Range.range_mm);
}

void send_lux(void)
{
	unsigned char data[] = {
		(unsigned char) Als.lux>>24,
		(unsigned char) Als.lux>>16,
		(unsigned char) Als.lux>>8,
		(unsigned char) Als.lux & 0x000000FF
	};
	send_can(ID_LUX_RANGE_CARD, ID_IHM, 'L', data, (int) sizeof(data) / sizeof(data[0]));
	term_printf("Lux: %d lux\n", Als.lux);
}

void send_axes(void)
{
	unsigned char data[] = {
		(unsigned char) ( (int) (phi) )>>8,
		(unsigned char) ( (int) (phi) )& 0xFF,
		(unsigned char) ( (int) (psi) )>>8,
		(unsigned char) ( (int) (psi) ) & 0xFF,
		(unsigned char) ( (int) (teta) )>>8,
		(unsigned char) ( (int) (teta) ) & 0xFF
	};
	send_can(ID_IMU_CARD, ID_IHM, 'A', data, (int) sizeof(data) / sizeof(data[0]));
    term_printf("roulis = %f \n", phi);
	term_printf("lacet = %f \n", psi);
	term_printf("tangage = %f \n", teta);
}

// END SEND FUNCTIONS
////////////////////////////////

////////////////////////////////
// RECEIVE FUNCTION

MsgRcv receive_can(void)
{
	CAN_Message msg_rcv;
	int lenMsg;
	lenMsg = can_Read(&msg_rcv);

	unsigned char data[6];
	for (int i=2; i<lenMsg; i++) {
		data[i-2] = msg_rcv.data[i];
	}

	MsgRcv m;
	m.fromId = msg_rcv.data[0];
	m.toId 	=  msg_rcv.id;
	m.len 	= lenMsg-2;
	m.data 	= data;
	m.order = msg_rcv.data[1];
	return m;
}
// END RECEIVE FUNCTION
////////////////////////////////


//====================================================================
//			CAN CALLBACK RECEPT
//====================================================================

void can_callback(void)
{
//envoi des donn??es CAN
	MsgRcv msg_rcv;
	msg_rcv = receive_can();

	term_printf("fromID %x\n", (int)msg_rcv.fromId);
	term_printf("toId %x\n", (int)msg_rcv.toId);
	term_printf("data %d\n", (int)msg_rcv.data);
	term_printf("len %x\n", (int)msg_rcv.len);
	term_printf("order %x\n", (int)msg_rcv.order);



	if (msg_rcv.fromId == (unsigned char) ID_IHM)
	{
	term_printf("yolo\n");
		switch(msg_rcv.order)
		{
#if MPL115A_ANEMO
		case (unsigned char) 'P':
			send_pressure();
			break;
		case (unsigned char) 'T':
			send_temperature();
			break;
		case (unsigned char) 'W':
			send_wind_speed();
			break;
		case (unsigned char) 'B':
			send_pressure_temperature_wind_speed();
			break;
#endif
#if VL6180X
		case (unsigned char) 'D':
			if (State.mode == RunAlsPoll) send_lux();
			if (State.mode == RunRangePoll) send_distance();			
			break;
		case (unsigned char) 'X':
			term_printf("X = %d", (uint8_t) msg_rcv.data[0]);
			State.mode = ((uint8_t) msg_rcv.data[0]) ? RunAlsPoll : RunRangePoll;
			break;
#endif
#if MPU9250
		case (unsigned char) 'A':
			send_axes();
			break;
#endif
		}
	}
}

//====================================================================
//			TIMER CALLBACK PERIOD
//====================================================================

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    anemo_ResetCount();
	//term_printf("Anemometer speed: %d km/h\n", (int)anemo_speed);
    //term_printf("Pressure: %d kPa\n", (int)pressure);

}
//====================================================================

void VL6180x_Init(void)
{
	uint8_t id;
	State.mode = 1;

    XNUCLEO6180XA1_Init();
    HAL_Delay(500); // Wait
    // RESET
    XNUCLEO6180XA1_Reset(0);
    HAL_Delay(10);
    XNUCLEO6180XA1_Reset(1);
    HAL_Delay(1);

    HAL_Delay(10);
    VL6180x_WaitDeviceBooted(theVL6180xDev);
    id=VL6180x_Identification(theVL6180xDev);
    term_printf("id=%d, should be 180 (0xB4) \n\r", id);
    VL6180x_InitData(theVL6180xDev);

    State.InitScale=VL6180x_UpscaleGetScaling(theVL6180xDev);
    State.FilterEn=VL6180x_FilterGetState(theVL6180xDev);

     // Enable Dmax calculation only if value is displayed (to save computation power)
    VL6180x_DMaxSetState(theVL6180xDev, DMaxDispTime>0);

    switch_state=-1 ; // force what read from switch to set new working mode
    State.mode = AlrmStart;
}
//====================================================================
void VL6180x_Step(void)
{
    DISP_ExecLoopBody();

    new_switch_state = XNUCLEO6180XA1_GetSwitch();
    if (new_switch_state != switch_state) {
        switch_state=new_switch_state;
        status = VL6180x_Prepare(theVL6180xDev);
        // Increase convergence time to the max (this is because proximity config of API is used)
        VL6180x_RangeSetMaxConvergenceTime(theVL6180xDev, 63);
        if (status) {
            AbortErr("ErIn");
        }
        else{
            if (switch_state == SWITCH_VAL_RANGING) {
                VL6180x_SetupGPIO1(theVL6180xDev, GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT, INTR_POL_HIGH);
                VL6180x_ClearAllInterrupt(theVL6180xDev);
                State.ScaleSwapCnt=0;
                DoScalingSwap( State.InitScale);
            } else {
                 State.mode = RunAlsPoll;
                 InitAlsMode();
            }
        }
    }

    switch (State.mode) {
    case RunRangePoll:
        RangeState();
        //send_distance();
        break;

    case RunAlsPoll:
        AlsState();
        //send_lux();
        break;

    case InitErr:
        TimeStarted = g_TickCnt;
        State.mode = WaitForReset;
        break;

    case AlrmStart:
       GoToAlaramState();
       break;

    case AlrmRun:
        AlarmState();
        break;

    case FromSwitch:
        // force reading swicth as re-init selected mode
        switch_state=!XNUCLEO6180XA1_GetSwitch();
        break;

    case ScaleSwap:

        if (g_TickCnt - TimeStarted >= ScaleDispTime) {
            State.mode = RunRangePoll;
            TimeStarted=g_TickCnt; /* reset as used for --- to er display */
        }
        else
        {
        	DISP_ExecLoopBody();
        }
        break;

    default: {
    	 DISP_ExecLoopBody();
          if (g_TickCnt - TimeStarted >= 5000) {
              NVIC_SystemReset();
          }
    }
    }
}
//====================================================================

