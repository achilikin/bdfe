/*  BSD License
    Copyright (c) 2015 Andrey Chilikin https://github.com/achilikin

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
	Basic Linux I2C wrapper for Raspberry Pi and Intel Edison,
	only write is supported

	For Raspberry Pi make sure that RPi i2c driver is enabled, check following
	files:
	/etc/modprobe.d/raspi-blacklist.conf
	#blacklist i2c-bcm2708
	/etc/modules
	i2c-dev
	/etc/modprobe.d/i2c.conf
	options i2c_bcm2708 baudrate=400000
*/

#ifndef DISABLE_I2C

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "li2c.h"

/* by default RPi mode, so bus 1 is valid for RPi and Edison */
static li2c_mode_t i2c_mode = LI2C_RPI;

/** i2c bus file descriptors */
static int i2c_bus[LI2C_MAX_BUS];

static int is_valid_bus(uint8_t bus)
{
	if (i2c_mode == LI2C_RPI) {
		if (bus > PI2C_MAX_BUS)
			return 0;
		return 1;
	}
	if (i2c_mode == LI2C_EDISON) {
		if (bus > EDI2C_MAX_BUS)
			return 0;
		return 1;
	}
	return 0;
}

/* select I2C mode - Raspberry or Edison for bus index validation */
int li2c_init(li2c_mode_t mode)
{
	for(int i = 0; i < LI2C_MAX_BUS; i++)
		i2c_bus[i] = -1;
	i2c_mode = mode;
	return 0;
}

/** open I2C bus if not opened yet and store file descriptor */
int li2c_open(uint8_t bus)
{
	char bus_name[64];

	if (!is_valid_bus(bus))
		return -1;

	// already opened?
	if (i2c_bus[bus] >= 0)
		return 0;

	// open i2c bus and store file descriptor
	sprintf(bus_name, "/dev/i2c-%u", bus);

	if ((i2c_bus[bus] = open(bus_name, O_RDWR)) < 0)
		return -1;

	return 0;
}

/** close I2C bus file descriptor */
int li2c_close(uint8_t bus)
{
	if (!is_valid_bus(bus))
		return -1;

	if (i2c_bus[bus] >= 0)
		close(i2c_bus[bus]);
	i2c_bus[bus] = -1;

	return 0;
}

/** select I2C device for li2c_write() calls */
int li2c_select(uint8_t bus, uint8_t slave)
{
	if (!is_valid_bus(bus) || (i2c_bus[bus] < 0))
		return -1;

	return ioctl(i2c_bus[bus], I2C_SLAVE, slave);
}

/** write to I2C device selected by li2c_select() */
int li2c_write(uint8_t bus, const uint8_t *data, uint32_t len)
{
	if (!is_valid_bus(bus) || (i2c_bus[bus] < 0))
		return -1;

	if (write(i2c_bus[bus], data, len) != (ssize_t)len)
		return -1;

	return 0;
}

#endif
