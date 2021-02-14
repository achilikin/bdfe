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
	Limited set of functions for SSD1306 / SH1106 compatible OLED 128x64 displays
	in text mode to minimize memory footprint if used on Atmel AVRs chips
	with low memory.
*/

#ifndef DISABLE_I2C

#include <stdio.h>

#include "ossd_i2c.h"

#if (OSSD_TARGET == OSSD_IF_AVR)
	#include <avr/io.h>
	#include <avr/pgmspace.h>
	#include <util/atomic.h>
	#include "i2cmaster.h"
#else
	#define PROGMEM
	#define pgm_read_byte(x) (*((uint8_t *)x))
	#include <alloca.h>
	#include <memory.h>
	#if (OSSD_TARGET == OSSD_IF_WIRE)
		#include "wi2c.h"
	#else
	#include "li2c.h"
#endif
#endif

#define OSSD_CMD  0x00
#define OSSD_DATA 0x40

#define OSSD_SET_ADDR_MODE   0x20
#define OSSD_ADDR_MODE_PAGE  0x10
#define OSSD_ADDR_MODE_HOR   0x00
#define OSSD_ADDR_MODE_VER   0x01

#define OSSD_SET_COL_ADDR    0x21
#define OSSD_SET_PAGE_ADDR   0x22

#define OSSD_SET_START_LINE  0x40

#define OSSD_SET_START_PAGE  0xB0
#define OSSD_SET_START_LCOL  0x00
#define OSSD_SET_START_HCOL  0x10

#define OSSD_SET_MUX_RATIO   0xA8
#define OSSD_SET_DISP_OFFSET 0xD3

#define OSSD_SET_CONTRAST	 0x81

#define OSSD_SET_SEG_REMAP   0xA0
#define OSSD_SEG_REMAP_R2L   0x01

#define OSSD_SET_COM_DIR     0xC0
#define OSSD_COM_DIR_UPDOWN  0x08

#define OSSD_SET_COM_CONFIG  0xDA
#define OSSD_COM_ALT         0x12
#define OSSD_COM_LR_REMAP    0x22

#define OSSD_SET_SLEEP_ON	 0xAE
#define OSSD_SET_SLEEP_OFF	 0xAF

#define OSSD_SET_INVERSE_ON	 0xA7
#define OSSD_SET_INVERSE_OFF 0xA6

#define OSSD_SET_OUTPUT_RAM	 0xA4
#define OSSD_SET_OUTPUT_ON	 0xA5

#define OSSD_SET_DISP_CLOCK  0xD5

#define OSSD_SET_PRECHARGE   0xD9

#define OSSD_SET_VCOMH_LEVEL 0xDB
#define OSSD_VCOMH_L065      0x00
#define OSSD_VCOMH_L077      0x20
#define OSSD_VCOMH_L083      0x30

#define OSSD_SET_CHARGE_PUMP 0x8D
#define OSSD_CHARGE_PUMP_ON  0x14
#define OSSD_CHARGE_PUMP_OFF 0x10

#define OSSD_SET_DC_DC       0xAD
#define OSSD_DC_DC_DISABLE   0x8A
#define OSSD_DC_DC_ENABLE    0x8B

#define OSSD_SET_PUMP_VPP    0x30
#define OSSD_VPP_V74         0x00 // 7.4V
#define OSSD_VPP_V80         0x01 // 8.0V
#define OSSD_VPP_V84         0x02 // 8.4V
#define OSSD_VPP_V90         0x03 // 9.0V

static const uint8_t font68[] PROGMEM = {
#include "font6x8.h"
};

static const uint8_t font88[] PROGMEM = {
#include "font88.h"
};

static const uint8_t font816[] PROGMEM = {
#include "font816.h"
};

static ossd_font_t _ofont[OSSD_FONT_MAX+1] = {
	{  6,  8, 32, 127-32, font68 },
	{  8,  8, 32, 127-32, font88 },
	{  8, 16, 32, 127-32, font816 },
	{  0,  0, 0,       0, NULL }
};

static uint8_t _cfont;
static uint8_t _offset;
static uint8_t _i2c_val;

uint8_t ossd_select_font(uint8_t font)
{
	uint8_t fret = _cfont;
	if (font <= OSSD_FONT_MAX)
		_cfont = font;
	return fret;
}

void ossd_set_user_font(ossd_font_t *nfont, ossd_font_t *ofont)
{
	if (ofont) {
		ofont->gw = _ofont[OSSD_FONT_USER].gw;
		ofont->gh = _ofont[OSSD_FONT_USER].gh;
		ofont->go = _ofont[OSSD_FONT_USER].go;
		ofont->gn = _ofont[OSSD_FONT_USER].gn;
		ofont->font = _ofont[OSSD_FONT_USER].font;
	}
	_ofont[OSSD_FONT_USER].gw = nfont->gw;
	_ofont[OSSD_FONT_USER].gh = nfont->gh;
	_ofont[OSSD_FONT_USER].go = nfont->go;
	_ofont[OSSD_FONT_USER].gn = nfont->gn;
	_ofont[OSSD_FONT_USER].font = nfont->font;
}

#if (OSSD_TARGET == OSSD_IF_LINUX)

static void ossd_send_byte(uint8_t dc, uint8_t data)
{
	uint8_t buf[2];
	buf[0] = dc;
	buf[1] = data;

	li2c_write(_i2c_val, buf, 2);
}

static void ossd_cmd_arg(uint8_t cmd, uint8_t arg)
{
	uint8_t data[3];
	data[0] = OSSD_CMD;
	data[1] = cmd;
	data[2] = arg;
	li2c_write(_i2c_val, data, 3);
}

static void ossd_fill_line(uint8_t data, uint8_t num)
{
	uint8_t *buf = (uint8_t *)alloca(num+1);
	memset(buf, data, num+1);
	buf[0] = OSSD_DATA;
	li2c_write(_i2c_val, buf, num+1);
}

#else

static void ossd_send_byte(uint8_t dc, uint8_t data)
{
	i2c_start(_i2c_val);
	i2c_write(dc);
	i2c_write(data);
	i2c_stop();
}

static void ossd_cmd_arg(uint8_t cmd, uint8_t arg)
{
	i2c_start(_i2c_val);
	i2c_write(OSSD_CMD);
	i2c_write(cmd);
	i2c_write(arg);
	i2c_stop();
}

#if (OSSD_TARGET == OSSD_IF_AVR)

static void ossd_fill_line(uint8_t data, uint8_t num)
{
	uint8_t i;
	i2c_start(_i2c_val);
	i2c_write(OSSD_DATA);
	for(i = 0; i < num; i++)
		i2c_write(data);
	i2c_stop();
}

#else

static inline void ossd_data(uint8_t cmd);

static void ossd_fill_line(uint8_t data, uint8_t num)
{
	uint8_t i;
	for(i = 0; i < num; i++)
		ossd_data(data);
}
#endif

#endif

static inline void ossd_cmd(uint8_t cmd)
{
	ossd_send_byte(OSSD_CMD, cmd);
}

static inline void ossd_data(uint8_t data)
{
	ossd_send_byte(OSSD_DATA, data);
}

void ossd_goto(uint8_t line, uint8_t x)
{
	x += _offset;
	ossd_cmd(OSSD_SET_START_PAGE | (line & 0x07));
	ossd_cmd(OSSD_SET_START_LCOL | (x & 0x0F));
	ossd_cmd(OSSD_SET_START_HCOL | (x >> 4));
}

void ossd_fill_screen(uint8_t data)
{
	// fill full screen line by line
	uint8_t line;
	for(line = 0; line < 8; line++) {
		ossd_goto(line, 0);
		ossd_fill_line(data, 128);
	}
}

void ossd_sleep(uint8_t on_off)
{
	if (on_off)
		ossd_cmd(OSSD_SET_SLEEP_ON);
	else
		ossd_cmd(OSSD_SET_SLEEP_OFF);
}

void ossd_set_contrast(uint8_t val)
{
	ossd_cmd_arg(OSSD_SET_CONTRAST, val);
}

static void ossd_put_centre(uint8_t line, const char *str, uint8_t atr)
{
	uint16_t len;
	uint8_t x = 0;
	uint8_t gw = _ofont[_cfont].gw;
	uint8_t gh = _ofont[_cfont].gh;
	for(len = 0; str[x]; len+=gw, x++);
	if (len > 128)
		x = 0;
	else
		x = (128 - len) / 2;

	// in case if new text is shorter than previous one
	// we clean line up to x position
	if (x) {
		uint8_t i;
		for(i = 0; i < (gh+7)/8; i++) {
			ossd_goto(line + i, 0);
			ossd_fill_line(0, x);
		}
	}

	// recursive call of ossd_putlx()
	ossd_putlx(line, x, str, atr);

	// in case if new text is shorter than previous one
	// we clean to the end of the line
	if (x) {
		uint8_t i;
		for(i = 0; i < (gh+7)/8; i++) {
			ossd_goto(line + i, x + len);
			ossd_fill_line(0, x);
		}
	}
}

void ossd_putlx(uint8_t line, int8_t x, const char *str, uint8_t atr)
{
	line &= 0x07;

	// try to put this text in the middle of the line:
	// ossd_put_centre() will calculate proper x coordinate
	if (x < 0) {
		ossd_put_centre(line, str, atr);
		return;
	}

	uint8_t rev = 0;
	uint8_t over = 0;
	uint8_t under = 0;
	if (atr & OSSD_TEXT_REVERSE)
		rev = ~rev;
	if (atr & OSSD_TEXT_OVERLINE)
		over = 0x01;
	if (atr & OSSD_TEXT_UNDERLINE)
		under = 0x80;

	uint8_t gw = _ofont[_cfont].gw;
	uint8_t gh = _ofont[_cfont].gh;
	uint8_t go = _ofont[_cfont].go;
	uint8_t gb = gw*(gh / 8); // bytes per glyph
	const uint8_t *font = _ofont[_cfont].font;
	for(; *str != '\0'; str++, x += gw) {
		uint16_t idx = (*str - go) * gb;
		if ((uint8_t)x > (128 - gw)) {
			x = 0;
			line = (line + (gh+7)/8) & 0x07;
		}
		ossd_goto(line, x);
		uint8_t i;
		for(i = 0; i < gb; i++) {
			if ((gh > 8) && (i == 8))
				ossd_goto(line + 1, x);
			uint8_t d = pgm_read_byte(&font[idx+i]);
			d ^= rev;
			if (under && (gh == 8 || i > (gw - 1)))
				d ^= under;
			if (i < gw)
				d ^= over;
			ossd_data(d);
		}
	}
}

void ossd_init(uint8_t driver, uint8_t i2c_val, uint8_t orientation)
{
	_offset = 0;
	_i2c_val = i2c_val;
#if (OSSD_TARGET == OSSD_IF_AVR)
	_i2c_val = (_i2c_val << 1) | I2C_WRITE;
#endif
	/* set all default values */
	ossd_cmd(OSSD_SET_SLEEP_ON);
	ossd_cmd_arg(OSSD_SET_MUX_RATIO, 63);
	ossd_cmd_arg(OSSD_SET_DISP_OFFSET, 0);
	ossd_cmd(OSSD_SET_START_PAGE);
	ossd_cmd(OSSD_SET_START_LINE | 0);
	ossd_cmd(OSSD_SET_START_LCOL | 2);
	ossd_cmd(OSSD_SET_START_HCOL | 0);
	ossd_cmd(OSSD_SET_SEG_REMAP | (orientation & OSSD_SEG_REMAP_R2L));
	ossd_cmd(OSSD_SET_COM_DIR | (orientation & OSSD_COM_DIR_UPDOWN));
	ossd_cmd_arg(OSSD_SET_COM_CONFIG, OSSD_COM_ALT);
	ossd_cmd_arg(OSSD_SET_CONTRAST, 0x7F);
	ossd_cmd(OSSD_SET_OUTPUT_RAM);
	ossd_cmd_arg(OSSD_SET_DISP_CLOCK, 0x80);
	ossd_cmd_arg(OSSD_SET_PRECHARGE, 0x22);
	ossd_cmd_arg(OSSD_SET_VCOMH_LEVEL, OSSD_VCOMH_L077);
	ossd_cmd(OSSD_SET_INVERSE_OFF);

	if (driver != OSSD_SH1106)
		ossd_cmd_arg(OSSD_SET_CHARGE_PUMP, OSSD_CHARGE_PUMP_ON);
	else {
		ossd_cmd_arg(OSSD_SET_DC_DC, OSSD_DC_DC_ENABLE); /* POR */
		ossd_cmd(OSSD_SET_PUMP_VPP | OSSD_VPP_V74);
		_offset = 2;
	}

	ossd_fill_screen(0x00);
	ossd_cmd(OSSD_SET_SLEEP_OFF);
	ossd_goto(0, 0);
}

#endif
