#ifndef _BME280_H
#define _BME280_H

#include <stdint.h>
#include "I2C.h"

// BME280 Register Addresses
#define BME280 								 0x76 // or 0x77 depending on SDO pin
#define BME280_REG_DATA				 0xF7
#define BME280_REG_CONFIG      0xF5
#define BME280_REG_CTRL_MEAS   0xF4
#define BME280_REG_CTRL_HUM    0xF2
#define BME280_REG_STATUS      0xF3
#define BME280_REG_RESET       0xE0
#define BME280_REG_ID          0xD0

//Calibration Register Addresses

//Temperature
#define DIG_T1_ADDR						0x88
#define DIG_T2_ADDR						0x8A
#define DIG_T3_ADDR						0x8C

//Pressure
#define DIG_P1_ADDR						0x08E
#define DIG_P2_ADDR						0x090
#define DIG_P3_ADDR						0x092
#define DIG_P4_ADDR						0x094
#define DIG_P5_ADDR						0x096
#define DIG_P6_ADDR						0x098
#define DIG_P7_ADDR						0x09A
#define DIG_P8_ADDR						0x09C
#define DIG_P9_ADDR						0x09E

//Humidity
#define DIG_H1_ADDR						0xA1
#define DIG_H2_ADDR						0xE1
#define DIG_H3_ADDR						0xE3
#define DIG_H4_ADDR						0xE4
#define DIG_H5_ADDR						0xE5
#define DIG_H6_ADDR						0xE7

/*BME specific functions*/

uint8_t bme280_init(void);
void bme280_config(void);
int32_t read_temperature(void);
uint32_t read_pressure(void);
uint32_t read_humidity(void);

#endif