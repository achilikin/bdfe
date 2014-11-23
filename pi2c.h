/*  BSD License
    Copyright (c) 2014 Andrey Chilikin https://github.com/achilikin

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
	BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

/**
	Basic I2C wrapper for Raspberry Pi, only write is supported.

	Make sure that RPi i2c driver is enabled, check following files:
	/etc/modprobe.d/raspi-blacklist.conf
		#blacklist i2c-bcm2708
	/etc/modules
		i2c-dev
	/etc/modprobe.d/i2c.conf
		options i2c_bcm2708 baudrate=400000
*/

#ifndef __PI2C_H__
#define __PI2C_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI2C_BUS0 0 /*< P5 header I2C bus */
#define PI2C_BUS1 1 /*< P1 header I2C bus */
#define PI2C_BUS  PI2C_BUS1 // default bus

int pi2c_open(uint8_t bus);  /*< open I2C bus  */
int pi2c_close(uint8_t bus); /*< close I2C bus */
int pi2c_select(uint8_t bus, uint8_t slave); /*< select I2C slave */
int pi2c_write(uint8_t bus, const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
