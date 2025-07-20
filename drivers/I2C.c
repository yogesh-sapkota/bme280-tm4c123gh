#include <stdlib.h>
#include "I2C.h"
#include "TM4C123.h"

void i2c0_init(void){
    SYSCTL->RCGCGPIO |= 0x02;   // enable clock to GPIO Port B 
    SYSCTL->RCGCI2C  |= 0x01;   // enable I2C0 Module
    
    // Wait for clocks to be ready
    while((SYSCTL->RCGCGPIO & 0x02) == 0){}; 
    //while((SYSCTL->RCGCI2C & 0x01) == 0){};
    
    GPIOB->DEN |= 0x0C;         // enable digital I/O on PB2 and PB3
		/* need to add disable analog functions? */
    GPIOB->AFSEL |= 0x0C;       // enable alternate function on PB2 (SCL) and PB3 (SDA)
    GPIOB->ODR   |= 0x08;       // enable open-drain for SDA
    GPIOB->PCTL  |= 0x3300;     // enable I2C on PB2 and PB3 pins
    I2C0->MCR    |= 0x10;       // master function enable 
    
    /* 
        TPR = (System Clock/(2*(SCL_LP + SCL_HI)*SCL_Frequency))-1;
        TPR = (80MHz/(2*(6+4)*100000))-1;
        TPR = 40-1 = 39 (0x27)
    */
    I2C0->MTPR  |= 0x27;        // SCL 100kbps speed for 80MHz system clock
}

// Wait for I2C operation to complete with timeout
bool i2c0_wait_for_completion(void) {
    uint32_t timeout = I2C_TIMEOUT;
    
    while((I2C0->MCS & I2C_MCS_BUSY) && (timeout > 0)) {
        timeout--;
    }
    
    return (timeout > 0); // Return true if operation completed, false if timeout
}

// Reset I2C module
void i2c0_reset(void) {
    while(I2C0->MCR & I2C_MCS_BUSY); // Wait for reset to complete
}

/*reads BME280 device id*/

uint8_t read_device_id(void){
    uint8_t deviceID = 0xFF; // Default error value
    
    // Reset I2C module first
    i2c0_reset();
    
    // Step 1: Send device address + write bit
    I2C0->MSA = (BME280 << 1) | 0;  											 // set device address and transmit  
    I2C0->MDR = 0xD0;               											 // device ID register address
    I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN;               // enable Master to transmit data and generate START
    
    // Wait for transmission with timeout
    if (!i2c0_wait_for_completion()) {
        return 0xEE; // Timeout error
    }
    
    // Check for errors
    if (I2C0->MCS & 0x02) { // Check for error bit
        return 0xDD; // I2C error
    }
    
    // Step 2: Send device address + read bit and receive data
    I2C0->MSA = (BME280 << 1) | 1;  // set device address and receive
    I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;               // start condition followed by receive and STOP condition
    
    // Wait for reception with timeout
    if (!i2c0_wait_for_completion()) {
        return 0xCC; // Timeout error
    }
    
    // Check for errors
    if (I2C0->MCS & 0x02) { // Check for error bit
        return 0xBB; // I2C error
    }
    
    deviceID = I2C0->MDR & 0xFF; // Read the device ID
    
    return deviceID;
} 

// returns 0 if successful
uint32_t i2c0_write_multiple_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint8_t length){
	if(data == NULL || length == 0)
		return 0x01; // Invalid parameters
	
	i2c0_reset();
	
	//Step 1: Send device address + write bit
  I2C0->MSA = (device_addr << 1) | 0;  							// set device address and transmit  
	I2C0->MDR = reg_addr & 0xFF;      				  			// device ID register
	I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN;  	      // enable Master to transmit data and generate START
	
	//Wait for transmission to be done
	if(!i2c0_wait_for_completion())
		return I2C_MCS_ERROR ;
	
	// Check for Error bit
	if(I2C0->MCS & I2C_MCS_ERROR){
		I2C0->MCS = I2C_MCS_STOP;
		return (I2C0->MCS & I2C_MCS_ERROR);
	}
	
	//Step 2: Send data bytes
	for(uint8_t i=0; i < length; i++){
		
		I2C0->MDR = data[i] & 0xFF;
		
		if(i == length-1){
			I2C0->MCS = I2C_MCS_RUN | I2C_MCS_STOP; // Last byte,send stop
		}
		else{
			I2C0->MCS = I2C_MCS_RUN; 							  // Transmit only
		}		
		
	// Wait for transmission with timeout
  if(!i2c0_wait_for_completion()) {
       return 0x04; // Timeout error
   }
        
	// Check for errors
	if(I2C0->MCS & I2C_MCS_ERROR) {  // Check for error bit
			I2C0->MCS = I2C_MCS_STOP ;   // Send stop
       return 0x05;        				 // I2C error
     }  
	}
    return 0; // Success
}

uint32_t i2c0_read_multiple_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint8_t length){

	//Step 1: Send device address + write bit
 	if(data == NULL || length == 0)
		return I2C_ERROR_PARAM; // Invalid parameters
	
	i2c0_reset();
	
	//Step 1: Send device address + write bit
  I2C0->MSA = (device_addr << 1) | 0;  							// set device address and transmit  
	I2C0->MDR = reg_addr & 0xFF;      				  			// device ID register
	I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN;  	      // enable Master to transmit data and generate START
	
	//Wait for transmission to be done
	if(!i2c0_wait_for_completion())
		return I2C_MCS_ERROR ;
	
	// Check for Error bit
	if((I2C0->MCS & I2C_MCS_ERROR)){
		I2C0->MCS = I2C_MCS_STOP;
		return (I2C0->MCS & I2C_MCS_ERROR);
	}
	
	//Step 2: Send device address + read bit and receive data, 
	I2C0->MSA = (device_addr << 1) | 1;
	
	for(uint8_t i=0; i < length; i++){
		
		if(i == 0 && length == 1){																// Single byte read
			I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN; 								// Start, Transmit, Stop
		}
		else if(i==0){ 																						// Repeated START
			I2C0->MCS = I2C_MCS_ACK | I2C_MCS_START | I2C_MCS_RUN;  // Start Condition of Multibyte receive
		}
		else if (i < length-1){
			I2C0->MCS = I2C_MCS_ACK | I2C_MCS_RUN; 									// Middle bytes, TRANSMIT + ACK
		}
		else{
			I2C0->MCS = I2C_MCS_STOP | I2C_MCS_RUN;                 // Last byte, TRANSMIT + STOP
		}
			
	// Wait for transmission with timeout
  if(!i2c0_wait_for_completion()) {
       return 0x04; // Timeout error
   }
        
	// Check for errors
	if(I2C0->MCS & I2C_MCS_ERROR) {  // Check for error bit
			I2C0->MCS = I2C_MCS_STOP ;   // Send stop
       return 0x05;        				// I2C error
     }
	data[i] = I2C0->MDR & 0xFF;
  }
    
    return I2C_SUCCESS ; // Success
}




