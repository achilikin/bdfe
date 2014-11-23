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
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "pi2c.h"

/** i2c bus file descriptors */
static int i2c_bus[2] = { -1, -1 };

/** open I2C bus if not opened yet and store file descriptor */
int pi2c_open(uint8_t bus)
{
	char bus_name[64];

	if (bus > PI2C_BUS1)
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
int pi2c_close(uint8_t bus)
{
	if (bus > PI2C_BUS1)
		return -1;

	if (i2c_bus[bus] >= 0)
		close(i2c_bus[bus]);
	i2c_bus[bus] = -1;

	return 0;
}

/** select I2C device for pi2c_write() calls */
int pi2c_select(uint8_t bus, uint8_t slave)
{
	if ((bus > PI2C_BUS1) || (i2c_bus[bus] < 0))
		return -1;

	return ioctl(i2c_bus[bus], I2C_SLAVE, slave);
}

/** write to I2C device selected by pi2c_select() */
int pi2c_write(uint8_t bus, const uint8_t *data, uint32_t len)
{
	if ((bus > PI2C_BUS1) || (i2c_bus[bus] < 0))
		return -1;

	if (write(i2c_bus[bus], data, len) != (ssize_t)len)
		return -1;

	return 0;
}
