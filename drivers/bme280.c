#include "bme280.h"

/* BME280 specific internal functions */

	static int32_t t_fine;
	static uint16_t dig_T1;
	static int16_t  dig_T2;
	static int16_t  dig_T3;

	static uint16_t dig_P1;
	static int16_t  dig_P2;
	static int16_t  dig_P3;
	static int16_t  dig_P4;
	static int16_t  dig_P5;
	static int16_t  dig_P6;
	static int16_t  dig_P7;
	static int16_t  dig_P8;
	static int16_t  dig_P9;

	static uint8_t  dig_H1;
	static int16_t  dig_H2;
	static uint8_t  dig_H3;
	static int16_t  dig_H4;
	static int16_t  dig_H5;
	static uint8_t  dig_H6;


static uint32_t bme280_read_udata(uint8_t* data, uint8_t length); // Read uncompensated data
static void bme280_read_calib_coeff(void);												// Calibration Coefficients

static int32_t	bme280_compensate_T_int32(int32_t);
static int32_t bme280_read_temp_calib_data(uint8_t*,uint8_t);

static uint32_t bme280_compensate_P_int64(int32_t);
static int32_t bme280_read_pres_calib_data(uint8_t*,uint8_t);

static uint32_t bme280_compensate_H_int32(int32_t);
static int32_t bme280_read_humid_calib_data(uint8_t*,uint8_t);


/* Initialization from Self test Section of BME280 Data Sheet */
// returns BME280 Device ID
uint8_t bme280_init(void){
	// Perform soft reset
	uint8_t reset_cmd = 0xB6;
	if(i2c0_write_multiple_bytes(BME280,BME280_REG_RESET, &reset_cmd,1) == 0){ // Writes single byte
			uart0_tx_string("soft reset complete\r\n");
	}
	else
		uart0_tx_string("soft reset failed\r\n");
	//read and verify chip id
	return(read_device_id());
}


// BME280 configuration
// Configure over-sampling rate, filter coefficient and standby time

void bme280_config(void){
	//humidity oversampling x 1 = 0x01
	uint8_t osrs_h = 0x01;
	i2c0_write_multiple_bytes(BME280,BME280_REG_CTRL_HUM, &osrs_h,1);
	//pressure and temperature humidity oversampling x1 = 0x01;
	uint8_t osrs_t = 0x01;
	uint8_t osrs_p = 0x01;
	uint8_t power_mode = 0x03;
	uint8_t cntrl_meas_reg = (osrs_p << 5) | (osrs_t << 2) | power_mode;
	i2c0_write_multiple_bytes(BME280,BME280_REG_CTRL_MEAS, &cntrl_meas_reg,1);
}

//read uncompensated value
static uint32_t bme280_read_udata(uint8_t* udata, uint8_t length){
	if(i2c0_read_multiple_bytes(BME280,BME280_REG_DATA, udata,length) == 0)
		return 0;
	
	return 1;
}

static int32_t	bme280_compensate_T_int32(int32_t adc_T){
	int32_t var1,var2,T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
  var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * (adc_T>>4) - ((int32_t)dig_T1)))>>12)*((int32_t)(dig_T3)) >> 14;
	t_fine = var1 + var2;
	
	T = (t_fine*5+128) >> 8;
	return T;
}


static int32_t bme280_read_temp_calib_data(uint8_t *calib_data,uint8_t length){
	if(i2c0_read_multiple_bytes(BME280,DIG_T1_ADDR, calib_data,length) == 0)
		return 0;
	return 1;
}

//Returns pressure in Pa as uint32_t in Q24.8 format 
// Output value 24674867 represents 24674867/256 = 96386.2 Pa
static uint32_t bme280_compensate_P_int64(int32_t adc_P){
	  int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

static int32_t bme280_read_pres_calib_data(uint8_t *calib_data,uint8_t length){
	if(i2c0_read_multiple_bytes(BME280,DIG_P1_ADDR, calib_data,length) == 0)
		return 0;
	return 1;
}

// Returns humidity in %RH as unsigned 32 bit int in Q22.10 (22 integer & 10 fractional bits)
// Output value 47445 represents 47445/1024 = 46.333 %RH
static uint32_t bme280_compensate_H_int32(int32_t adc_H){
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - 
                    (((int32_t)dig_H5) * v_x1_u32r)) + 
                   ((int32_t)16384)) >> 15) * 
                 (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * 
                      (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + 
                       ((int32_t)32768))) >> 10) + 
                    ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                              ((int32_t)dig_H1)) >> 4);
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (uint32_t)(v_x1_u32r >> 12);
}

static int32_t bme280_read_humid_calib_data(uint8_t *calib_data,uint8_t length){
	if(i2c0_read_multiple_bytes(BME280,DIG_H2_ADDR, calib_data,length) == 0)
		return 0;
	return 1;
}

void bme280_read_calib_coeff(void){
	
	uint8_t temp_calib_data[8];
	uint8_t press_calib_data[20];
	uint8_t humid_calib_data[8];
	
	bme280_read_temp_calib_data(temp_calib_data,6);
	bme280_read_pres_calib_data(press_calib_data,18);
	bme280_read_humid_calib_data(humid_calib_data,8);
	
	dig_T1 = (uint16_t)(temp_calib_data[1]<<8 | temp_calib_data[0]);
	dig_T2 = (int16_t)(temp_calib_data[3]<<8 | temp_calib_data[2]);
	dig_T3 = (int16_t)(temp_calib_data[5]<<8 | temp_calib_data[4]);
	
	dig_P1 = (uint16_t)(press_calib_data[1]<<8 | press_calib_data[0]);
	dig_P2 = (int16_t)(press_calib_data[3]<<8 | press_calib_data[2]);
	dig_P3 = (int16_t)(press_calib_data[5]<<8 | press_calib_data[4]);
	dig_P4 = (int16_t)(press_calib_data[7]<<8 | press_calib_data[6]);
	dig_P5 = (int16_t)(press_calib_data[9]<<8 | press_calib_data[8]);
	dig_P6 = (int16_t)(press_calib_data[11]<<8 | press_calib_data[10]);
	dig_P7 = (int16_t)(press_calib_data[13]<<8 | press_calib_data[12]);
	dig_P8 = (int16_t)(press_calib_data[15]<<8 | press_calib_data[14]);
	dig_P9 = (int16_t)(press_calib_data[17]<<8 | press_calib_data[16]);
	
	dig_H1 = (uint8_t)(press_calib_data[18]);

	dig_H2 = (int16_t)(humid_calib_data[1]<<8 | humid_calib_data[0]);
	dig_H3 = (uint8_t)(humid_calib_data[2]);
	dig_H4 = (int16_t)(humid_calib_data[3]<<4 | humid_calib_data[4]);
	dig_H5 = (int16_t)((humid_calib_data[5]<<4) | (humid_calib_data[4]>>4));
	dig_H6 = (uint8_t)(humid_calib_data[6]);
}


int32_t read_temperature(void){
	
	uint8_t uncomp_data[8];
	int32_t temp;
	int32_t temp_comp;

	
	if(bme280_read_udata(uncomp_data,8) == 0){
			//uart0_tx_string("data read success...\r\n");
		}
	
	uint8_t temp_msb = uncomp_data[3];
	uint8_t temp_lsb = uncomp_data[4];
	uint8_t temp_xlsb = uncomp_data[5];
		
	bme280_read_calib_coeff();
		
	temp = ((int32_t)temp_msb<<12) | ((int32_t)temp_lsb<<4) | (temp_xlsb>>4);
	temp_comp = bme280_compensate_T_int32(temp);
		
	return temp_comp;

}

uint32_t read_pressure(void){
	
	uint8_t uncomp_data[8];
	int32_t pressure;
	uint32_t press_comp;

	if(bme280_read_udata(uncomp_data,8) == 0){
			//uart0_tx_string("data read success...\r\n");
		}

	uint8_t press_msb = uncomp_data[0];
	uint8_t press_lsb = uncomp_data[1];
	uint8_t press_xlsb = uncomp_data[2];
		
	bme280_read_calib_coeff();
	
	pressure = ((int32_t)press_msb<<12) | ((int32_t)press_lsb<<4) | (press_xlsb>>4);
	press_comp = bme280_compensate_P_int64(pressure);
		
	return (uint32_t) (press_comp/256);
}
	
uint32_t read_humidity(void){
	
	uint8_t uncomp_data[8];
	int32_t humidity;
	uint32_t humid_comp;

	if(bme280_read_udata(uncomp_data,8) == 0){
			//uart0_tx_string("data read success...\r\n");
		}
	
	uint8_t hum_msb = uncomp_data[6];
  uint8_t hum_lsb = uncomp_data[7];
		
	bme280_read_calib_coeff();
	
	humidity = (int32_t)(hum_msb<<8) | hum_lsb;
	humid_comp = bme280_compensate_H_int32(humidity);
		
	return (uint32_t) (humid_comp/1024);

}
