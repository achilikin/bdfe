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
#ifndef DISABLE_I2C

#ifndef __OLED_SSD1306_I2C__
#define __OLED_SSD1306_I2C__

/**
  Limited set of functions for SSD1306 / SH1106 compatible OLED 128x64 displays
  in text mode to minimize memory footprint if used on Atmel AVRs chips
  with low memory.
*/

#ifdef __cplusplus
extern "C" {
#if 0
} // dummy bracket for Visual Assist
#endif
#endif

/** Target interface */
#define OSSD_IF_AVR	  1 /*< AVR compiler */
#define OSSD_IF_LINUX 2 /*< Linux native Intel Edison or Raspberry Pi */
#define OSSD_IF_WIRE  3 /*< Arduino Wire Interface on Intel Edison */

#ifndef OSSD_TARGET
#ifdef __AVR_ARCH__
	#define OSSD_TARGET OSSD_IF_AVR
#else
	#define OSSD_TARGET OSSD_IF_LINUX
#endif
#endif

#if (OSSD_TARGET != OSSD_IF_AVR)
	#include <stdint.h>
#endif

#define OSSD_FONT_6x8  0
#define OSSD_FONT_8x8  1
#define OSSD_FONT_8x16 2
#define OSSD_FONT_USER 3
#define OSSD_FONT_MAX  OSSD_FONT_USER

typedef struct ossd_font_s
{
	uint8_t gw; /*< glyph width  */
	uint8_t gh; /*< glyph height */
	uint8_t go; /*< font offset, first glyph index */
	uint8_t gn; /*< number of glyphs presented */
	const uint8_t *font;
} ossd_font_t;

/**
  Most common OLED drivers SSD1306 and SH1106 are supported
  */
#define OSSD_SH1106  0
#define OSSD_SSD1306 1

/**
  flat cable connected at the top
  use ossd_init(OSSD_UPDOWN) to rotate screen
  */
#define OSSD_UPDOWN 0x09
#define OSSD_NORMAL 0x00

/**
  set default parameters
  driver: OSSD_SSD1306 or OSSD_SH1106
  i2c_val: I2C address for AVR
           I2C bus for Linux (Edison, RPi)
  */
void ossd_init(uint8_t driver, uint8_t i2c_val, uint8_t orientation);

/** fill screen with specified pattern */
void ossd_fill_screen(uint8_t data);

/** set display to sleep mode */
void ossd_sleep(uint8_t on_off);

/** set display contrast */
void ossd_set_contrast(uint8_t val);

/** select one of three fonts for following ossd_putlx() calls */
uint8_t ossd_select_font(uint8_t font);

/**
 set user font selectable by OSSD_FONT_USER to nfont
 store current user font in ofont (if not NULL)
 */
void ossd_set_user_font(ossd_font_t *nfont, ossd_font_t *ofont);

/** text attributes */
#define OSSD_TEXT_REVERSE   0x01
#define OSSD_TEXT_UNDERLINE 0x02
#define OSSD_TEXT_OVERLINE  0x04

/**
 output string up to 64 chars in length
 line: 0-7
 x:    0-127, or -1 for centre of the line
 str:  output string
 atr:  OSSD_TEXT_*
 */
void ossd_putlx(uint8_t line, int8_t x, const char *str, uint8_t atr);

/** void screen */
static inline void ossd_cls(void) {
	ossd_fill_screen(0);
}

#ifdef __cplusplus
}
#endif

#endif

#endif
