#include <Wire.h>

#include "wi2c.h"

int i2c_start(uint8_t address)
{
	Wire.beginTransmission(address);
}

int i2c_write(uint8_t data)
{
	Wire.write(data);
}

int i2c_stop(void)
{
	Wire.endTransmission();
}
