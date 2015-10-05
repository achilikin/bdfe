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
	Basic BDF Exporter - converts BDF files to C structures.
	Only 8 bits wide fonts are supported.
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bdf.h"
#include "li2c.h"
#include "rterm.h"
#include "ossd_i2c.h"

#define DISPLAY_FONT 0x80000000

/**
 sarg: short argument
 larg: long argument 
 */
static int arg_is(const char *arg, const char *sarg, const char *larg)
{
	if (sarg && (strcmp(arg, sarg) == 0))
		return 1;
	if (larg && (strcmp(arg, larg) == 0))
		return 1;
	return 0;
}

static void usage(const char *name)
{
	printf("%s [options] <bdf file>\n", name);
	printf("  options are:\n");
	printf("  header:     print file header\n");
	printf("  verbose:    add extra info to the header\n");
	printf("  line:       one line per glyph\n");
	printf("  subset a-b: subset of glyphs to convert a to b, default 32-126\n");
	printf("  all:        print all glyphs, not just 32-126\n");
	printf("  native:     do not adjust font height 8 pixels\n");
	printf("  ascender H: add extra ascender of H pixels per glyph\n");
	printf("  rotate:     rotate glyphs' bitmaps CCW\n");
	printf("  display     show converted font on SSD1306 compatible display\n");
	printf("  i2c_bus B:  I2C bus for SSD1306 compatible display (default 1)\n");
	printf("  i2c_addr A: I2C address for SSD1306 compatible display (default 0x3C)\n");
	printf("  updown:     display orientation is upside down\n");
}

int main(int argc, char **argv)
{
	char *file;
	bdfe_t *font;
  	int flags = 0;
	uint8_t i2c_bus = 1;
	uint8_t i2c_address = 0x3C;
	uint8_t orientation = 0;
	uint32_t gidx = 0;
	unsigned ascender = 0;
	unsigned gmin = 32, gmax = 126;

	if (argc < 2) {
		usage(basename(argv[0]));
		return -1;
	}

	for(int i = 1; i < argc; i++) {
		if (arg_is(argv[i], "-?", "help")) {
			usage(basename(argv[0]));
			return 0;
		}

		if (arg_is(argv[i], "-h", "header"))
			flags |= BDF_HEADER;

		if (arg_is(argv[i], "-v", "verbose"))
			flags |= BDF_VERBOSE;

		if (arg_is(argv[i], "-a", "ascender")) {
			if (i < argc && isdigit(*argv[i+1]))
				ascender = atoi(argv[++i]);
		}

		if (arg_is(argv[i], "-l", "line"))
			flags |= BDF_GPL;

		if (arg_is(argv[i], "-s", "subset")) {
			if (i < argc && isdigit(*argv[i+1])) {
				i++;
				char *end;
				gmin = strtoul(argv[i], &end, 10);
				gmax = gmin;
				if (*end == '-')
					gmax = strtoul(end+1, &end, 10);
				if (gmax < gmin) {
					unsigned tmp = gmin;
					gmin = gmax;
					gmax = tmp;
				}
			}
		}

		if (arg_is(argv[i], "-a", "all")) {
			gmin = 0;
			gmax = 0xFFFFFFFF;
		}

		if (arg_is(argv[i], "-n", "native"))
			flags |= BDF_NATIVE;

		if (arg_is(argv[i], "-r", "rotate"))
			flags |= BDF_ROTATE;

		if (arg_is(argv[i], "-B", "i2c_bus")) {
			i++;
			uint32_t i2bus = strtoul(argv[i], NULL, 16);
			if (i2bus && (i2bus < LI2C_MAX_BUS))
				i2c_bus = i2bus;
		}

		if (arg_is(argv[i], "-A", "i2c_addr")) {
			i++;
			uint32_t i2ca = strtoul(argv[i], NULL, 16);
			if (i2ca && (i2ca < 0x78))
				i2c_address = i2ca;
		}

		if (arg_is(argv[i], "-d", "display"))
			flags |= DISPLAY_FONT;

		if (arg_is(argv[i], "-u", "updown"))
			orientation = OSSD_UPDOWN;
	}

	file = argv[argc - 1];
	font = bdf_convert(file, gmin, gmax, ascender, flags);

	if (font == NULL) {
		fprintf(stderr, "Unable to convert '%s'\n", file);
		return -1;
	}

	if (!(flags & DISPLAY_FONT)) {
		free(font);
		return 0;
	}

	if (i2c_bus > PI2C_MAX_BUS)
		li2c_init(LI2C_EDISON);
	else
		li2c_init(LI2C_RPI);

	if (li2c_open(i2c_bus) < 0) {
		fprintf(stderr, "Unable to open i2c bus %d\n", i2c_bus);
		free(font);
		return -1;
	}
	if (li2c_select(i2c_bus, i2c_address) < 0) {
		fprintf(stderr, "Unable to open select device at %02X\n", i2c_address);
		free(font);
		return -1;
	}

	ossd_font_t of;
	of.gw   = font->gw;
	of.gh   = font->bpg;
	of.go   = (uint8_t)gmin;
	of.gn   = (uint8_t)font->chars;
	of.font = font->font;

	ossd_init(i2c_bus, orientation);
	ossd_set_user_font(&of, NULL);
	ossd_select_font(OSSD_FONT_USER);

	int gh = (of.gh + 7)/8; // glyph height in lines

	char buf[16];
	sprintf(buf, "%dx%d", of.gw, of.gh);
	file = basename(file);
	ossd_putlx(0, -1, file, OSSD_TEXT_REVERSE);
	ossd_putlx(8 - gh, -1, buf, OSSD_TEXT_UNDERLINE | OSSD_TEXT_OVERLINE);
	buf[1] = '\0';

	stdin_mode(TERM_MODE_RAW);
	fprintf(stderr, "Press any key to continue, 'q' to exit\n");
	if (stdin_getch(-1) == 'q')	goto exit;

	ossd_fill_screen(0);

	do {
		if (gidx > 0) {
			fprintf(stderr, "Press any key to continue, 'q' to exit\n");
			if (stdin_getch(-1) == 'q')	break;
			ossd_fill_screen(0);
		}
		for(int line = 0, l = 0; line < 8; line += gh, l++) {
			for(int i = 0; i < (128/of.gw) && gidx < font->chars; i++, gidx++) {
				buf[0] = gidx + of.go;
				ossd_putlx(line, i*of.gw, (const char *)buf, 0);
			}
		}
	} while(gidx < font->chars);

exit:
	stdin_mode(TERM_MODE_CAN);
	li2c_close(i2c_bus);
	free(font);

	return 0;
}
