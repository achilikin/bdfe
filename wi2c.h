#ifndef WIRE_TO_I2C_HEADER
#define WIRE_TO_I2C_HEADER

#ifdef __cplusplus
extern "C" {
#endif

int i2c_start(uint8_t address);
int i2c_write(uint8_t data);
int i2c_stop(void);

#ifdef __cplusplus
}
#endif

#endif
