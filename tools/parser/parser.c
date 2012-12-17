/* 
	HCIDump - HCI packet analyzer	
	Copyright (C) 2000-2001 Maxim Krasnyansky <maxk@qualcomm.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation;

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
	IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) AND AUTHOR(S) BE LIABLE FOR ANY CLAIM,
	OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
	RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
	NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
	USE OR PERFORMANCE OF THIS SOFTWARE.

	ALL LIABILITY, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PATENTS, COPYRIGHTS,
	TRADEMARKS OR OTHER RIGHTS, RELATING TO USE OF THIS SOFTWARE IS DISCLAIMED.
*/

/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <asm/types.h>

#include "parser.h"

struct parser_t parser;

void init_parser(unsigned long flags, unsigned long filter)
{
	parser.flags  = flags;
	parser.filter = filter;
	parser.state  = 0;
}

static inline void hex_dump(int level, unsigned char *buf, int len)
{
	register int i,n;

	for (i=0, n=1; i<len; i++, n++) {
		if (n == 1)
			p_indent(level, 0);
		printf("%2.2X ", buf[i]);
		if (n == DUMP_WIDTH) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n!=1)
		printf("\n");
}

static inline void ascii_dump(int level, unsigned char *buf, int len)
{
	register int i,n;

	for (i=0, n=1; i<len; i++, n++) {
		if (n == 1)
			p_indent(level, 0);
		printf("%1c ", isprint(buf[i]) ? buf[i] : '.');
		if (n == DUMP_WIDTH) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n!=1)
		printf("\n");
}

void raw_dump(int level, struct frame *frm)
{
	if (!frm->len)
		return;

	switch (parser.flags & DUMP_TYPE_MASK) {
	case DUMP_HEX:
		hex_dump(level, frm->ptr, frm->len);
		break;

	case DUMP_ASCII:
		ascii_dump(level, frm->ptr, frm->len);
		break;
	}
}
