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
#ifndef __BDF_EXPORTER_H__
#define __BDF_EXPORTER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#if 0
} // dummy bracket for Visual Assist
#endif
#endif

#define BDF_NATIVE   0x01 /*< use native height, do not adjust for SSD1306 */
#define BDF_ROTATE   0x02 /*< rotate glyphs CCW  */
#define BDF_MUTE     0x04 /*< do not print processed glyphs or header */
#define BDF_HEADER   0x08 /*< output file header */
#define BDF_VERBOSE  0x10 /*< verbose glyphs/header output */
#define BDF_GPL      0x20 /*< one line per glyph */


typedef struct bdfe_s
{
	unsigned gw;     /*< glyph width      */
	unsigned bpg;    /*< bytes per glyph  */
	unsigned chars;  /*< number of glyphs */
	uint8_t  font[]; /*< glyphs' bitmaps  */
} bdfe_t;

/**
 name      - bdf font name
 gmin/gmax - range of glyphs to be processed
 flags     - BDF_* above
 ascender  - extra ascender to be added, in pixels
 */
bdfe_t *bdf_convert(const char *name, unsigned gmin, unsigned gmax, unsigned ascender, int flags);

#ifdef __cplusplus
}
#endif

#endif
