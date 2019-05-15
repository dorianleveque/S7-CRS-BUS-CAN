/*
 * drv_spi.h
 *
 *  Created on: 29 févr. 2016
 *      Author: kerhoas
 */

#ifndef INC_DRV_SPI_H_
#define INC_DRV_SPI_H_


#include "main.h"


typedef struct _Coeff {
	float a1;
	float b1;
	float b2;
	float c12;
	} Coeff;

typedef struct _SPI1_Mesure {
	float pressure;
	float temperature;
	} SPI1_Mesure;

void spi1_Init(void);
uint8_t spi1_Transfer(uint8_t out);
void spi1_Transfer_cmd(uint8_t *cmd, int cmd_length, uint8_t *values);
Coeff spi1_get_coeff(void);
void spi1_start_pressure_temperature_conversion(void);
uint16_t spi1_get_adc_pressure(void);
uint16_t spi1_get_adc_temperature(void);
SPI1_Mesure spi1_get_pressure_temperature(void);


#endif /* INC_DRV_SPI_H_ */
