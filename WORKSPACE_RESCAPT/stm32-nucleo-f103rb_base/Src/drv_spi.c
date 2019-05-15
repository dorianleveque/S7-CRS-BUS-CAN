/*
 * drv_spi.c
 */


#include "drv_spi.h"

SPI_HandleTypeDef SpiHandle;

//=================================================================
//  SPI INIT
//=================================================================

void spi1_Init()
{
    SpiHandle.Instance               = SPI1;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE; //SPI_PHASE_2EDGE
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_HARD_OUTPUT ;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode = SPI_MODE_MASTER;

    HAL_SPI_Init(&SpiHandle);

    __HAL_SPI_ENABLE(&SpiHandle);
}

//=================================================================
//  SPI TRANSFER
//  Send OUT on MOSI, return data from MISO
//=================================================================

uint8_t spi1_Transfer(uint8_t out)
{
	uint8_t aTxBuffer[1];
	uint8_t aRxBuffer[1];

	aTxBuffer[0]=out;
	HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t *)aTxBuffer, (uint8_t *)aRxBuffer, 1, 100);
	return aRxBuffer[0];
}
//=========================================================================


//=================================================================
//  SPI TRANSFER CMD
//  Send a command list and return values of each command
//=================================================================
void spi1_Transfer_cmd(uint8_t *cmd, int cmd_length, uint8_t *values) {
	int sensor_result_index = 0;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0); // CS
	for (int i=0; i<cmd_length; i++){
		uint8_t value;
		value = spi1_Transfer(cmd[i]);
		if (i%2 == 1){ // au lieu de value != 0
			values[sensor_result_index] = value;
			//term_printf("SENSOR VALUE : %x \t| %d\n", sensor_result[sensor_result_index], sensor_result[sensor_result_index]);
			sensor_result_index++;
		}
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1); // |CS
}


//=================================================================
//  SPI GET COEFF
//  Return coefficient a1, b1, b2, c12
//=================================================================
Coeff spi1_get_coeff(void) {
	uint8_t cmd[] = {0x88, 0x00, 0x8A, 0x00, 0x8C, 0x00, 0x8E, 0x00, 0x90, 0x00, 0x92, 0x00, 0x94, 0x00, 0x96, 0x00, 0x00};
	int cmd_length = (int) sizeof(cmd) / sizeof(cmd[0]);

	uint8_t values[(int)(cmd_length/2)];
	spi1_Transfer_cmd(cmd, cmd_length, values);

	//term_printf("a0:  %x + %x  =  %x\n", values[0], values[1], ((values[0] << 8) | values[1]));
	Coeff coeffs = {
			/*((values[0] << 8) | values[1]),
			((values[2] << 8) | values[3]),
			((values[4] << 8) | values[5]),
			((values[6] << 8) | values[7]),*/
		(values[0] << 5) + (values[1] >> 3) + (values[1] & 0x07) / 8.0,
	    ((((values[2] & 0x1F) * 0x100)+values[3]) / 8192.0) - 3 ,
	    ((((values[4] - 0x80) << 8) + values[5]) / 16384.0 ) - 2,
	    (((values[6] * 0x100) + values[7]) / 16777216.0 )
		/*((float) ((values[0] << 8) + values[1]) / ((long)1 << 3)),
		((float) ((values[2] << 8) + values[3]) / ((long)1 << 13)),
		((float) ((values[4] << 8) + values[5]) / ((long)1 << 14)),
		((float) ((values[6] << 8) + values[7]) / ((long)1 << 24)),*/
	};
	return coeffs;
}


//=================================================================
//  SPI START PRESSURE TEMPERATURE CONVERSION
//=================================================================
void spi1_start_pressure_temperature_conversion(void) {
	uint8_t cmd[] = {0x24, 0x00};
	int cmd_length = (int) sizeof(cmd) / sizeof(cmd[0]);
	uint8_t values[1];
	spi1_Transfer_cmd(cmd, cmd_length, values);
}


//=================================================================
//  SPI GET ADC TEMPERATURE
// 	Return adc temperature
//=================================================================
uint16_t spi1_get_adc_temperature(void) {
	uint8_t cmd[] = {0x84, 0x00, 0x86, 0x00, 0x00};
	int cmd_length = (int) sizeof(cmd) / sizeof(cmd[0]);
	uint8_t values[(int)(cmd_length/2)];

	spi1_Transfer_cmd(cmd, cmd_length, values);

	uint16_t coeff = ((values[0] << 8) | values[1]);
	//uint16_t coeff = ((values[0] *256) + values[1])/64;
	return coeff;
}

//=================================================================
//  SPI GET ADC PRESSURE
// 	Return adc pressure
//=================================================================
uint16_t spi1_get_adc_pressure(void) {
	uint8_t cmd[] = {0x80, 0x00, 0x82, 0x00, 0x00};
	int cmd_length = (int) sizeof(cmd) / sizeof(cmd[0]);
	uint8_t values[(int)(cmd_length/2)];

	spi1_Transfer_cmd(cmd, cmd_length, values);

	uint16_t coeff = ((values[0] << 8) | values[1]);
	//uint16_t coeff = ((values[0] *256) + values[1])/64;
	return coeff;
}


//=================================================================
//  SPI GET PRESSURE
// 	Return pressure value of sensor
//=================================================================
SPI1_Mesure spi1_get_pressure_temperature(void) {
	Coeff coeffs;
	float a0;
	float b1;
	float b2;
	float c12;
	uint16_t Tadc;
	uint16_t Padc;
	float Pcomp;
	SPI1_Mesure m;

    coeffs = spi1_get_coeff();
    a0 = coeffs.a1;
    b1 = coeffs.b1;
    b2 = coeffs.b2;
    c12 =coeffs.c12;

    spi1_start_pressure_temperature_conversion();

    HAL_Delay(3);  // wait 3ms

    Padc = spi1_get_adc_pressure() >> 6;
    Tadc = spi1_get_adc_temperature() >> 6;



    Pcomp = a0 + (b1 + c12 * Tadc) * Padc + b2 * Tadc;

    m.pressure = (float) Pcomp * (65.0/1023.0) + 50.0;//( (1150.0-500.0)/1023.0 ) + 500.0;//(65.0/1023.0) + 50.0;

    m.temperature = (float)25.0 - ((float)Tadc - 512.0 ) / 5.35;
    // debug
/*
	term_printf("\n---------\n TRUE PRESSURE: \n");
    term_printf("a0:  %x\n", a0);
    term_printf("b1:  %x\n", b1);
    term_printf("b2:  %x\n", b2);
    term_printf("c12:  %x\n", c12);
    term_printf("Tadc:  %x\n", Tadc);
    term_printf("Padc:  %x\n", Padc);
    term_printf("Pcomp:  %x\n", Pcomp);*/
    //term_printf("Pressure:  %d kPa Temperature: %d °C\n", (int)pressure, (int)temperature);
	return m;
}