#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

#include "UART.h"

//TMC123GH I2C Registers
#define I2C_MCS_ACK             0x08  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x08  // Acknowledge Data
#define I2C_MCS_ADRACK          0x04  // Acknowledge Address
#define I2C_MCS_STOP            0x04  // Generate STOP
#define I2C_MCS_START           0x02  // Generate START
#define I2C_MCS_ERROR           0x02  // Error
#define I2C_MCS_RUN             0x01  // I2C Master Enable
#define I2C_MCS_BUSY            0x01  // I2C Busy
#define I2C_MCR_MFE             0x10  // I2C Master Function Enable

// Error codes (0 = success, non-zero = error)
#define I2C_SUCCESS          0x00
#define I2C_ERROR_PARAM      0x01  // Invalid parameters
#define I2C_ERROR_TIMEOUT    0x02  // Timeout error
#define I2C_ERROR_NACK       0x03  // No acknowledgment
#define I2C_ERROR_TIMEOUT2   0x04  // Timeout during data phase
#define I2C_ERROR_NACK2      0x05  // NACK during data phase
#define I2C_ERROR_BUSY       0x06  // Bus busy
#define I2C_ERROR_ARB        0x07  // Arbitration lost


// I2C timeout value (software delay)
#define I2C_TIMEOUT 10000

// Function prototypes
void i2c0_init(void);
uint8_t read_device_id(void);
bool i2c0_wait_for_completion(void);
void i2c0_reset(void);
uint32_t i2c0_write_multiple_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint8_t length);
uint32_t i2c0_read_multiple_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint8_t length);

#endif 