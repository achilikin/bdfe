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
	Sets stdin to raw mode so read() can get chars one by one 
	without waiting for the Enter
*/
#include <poll.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

#include "rterm.h"

static int restore = 0;

/** restore canonical mode ar exit */
static void	mode_restore_proc(void)
{
	stdin_mode(TERM_MODE_CAN);
}

/** set terminal mode: RAW or CANonical */
int stdin_mode(int mode)
{
	struct termios term_attr;

	tcgetattr(STDIN_FILENO, &term_attr);
	/* set the terminal to raw mode */
	if (mode == TERM_MODE_RAW) {
		term_attr.c_lflag &= ~(ECHO | ICANON);
		term_attr.c_cc[VMIN] = 0;
		if (!restore) {
			restore++;
			atexit(mode_restore_proc);
		}
	}
	else {
		term_attr.c_lflag |= (ECHO | ICANON);
		term_attr.c_cc[VMIN] = 1;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &term_attr);

	return mode;
}

/** read stdin with timeout in ms, or -1 until a key is pressed */
int stdin_getch(int timeout)
{
	struct pollfd polls;
	polls.fd = STDIN_FILENO;
	polls.events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
	polls.revents = 0;

	if (poll(&polls, 1, timeout) < 0)
		return -1;

	int ch = 0;
	if (polls.revents) {
		if (read(STDIN_FILENO, &ch, 1) < 0)
			return -1;
	}

	return ch;
}
