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
	CAPI parser.
	Copyright (C) 2004 Marcel Holtmann <marcel@holtmann.org>
*/

/*
 * $Id$
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>

#include "parser.h"


static char *cmd2str(uint8_t cmd)
{
	switch (cmd) {
	case 0x01:
		return "ALERT";
	case 0x02:
		return "CONNECT";
	case 0x03:
		return "CONNECT_ACTIVE";
	case 0x04:
		return "DISCONNECT";
	case 0x05:
		return "LISTEN";
	case 0x08:
		return "INFO";
	case 0x20:
		return "INTEROPERABILITY";
	case 0x41:
		return "SELECT_B_PROTOCOL";
	case 0x80:
		return "FACILITY";
	case 0x82:
		return "CONNECT_B3";
	case 0x83:
		return "CONNECT_B3_ACTIVE";
	case 0x84:
		return "DISCONNECT_B3";
	case 0x86:
		return "DATA_B3";
	case 0x87:
		return "RESET_B3";
	case 0xff:
		return "MANUFACTURER";
	default:
		return "UNKNOWN";
	}
}

static char *subcmd2str(uint8_t subcmd)
{
	switch (subcmd) {
	case 0x80:
		return "REQ";
	case 0x81:
		return "CONF";
	case 0x82:
		return "IND";
	case 0x83:
		return "RESP";
	default:
		return "UNKN";
	}
}

static char *interopsel2str(uint16_t sel)
{
	switch (sel) {
	case 0x0000:
		return "USB Device Management";
	case 0x0001:
		return "Bluetooth Device Management";
	default:
		return "Unknown";
	}
}

static char *func2str(uint16_t func)
{
	switch (func) {
	case 0:
		return "Register";
	case 1:
		return "Release";
	case 2:
		return "Get_Profile";
	case 3:
		return "Get_Manufacturer";
	case 4:
		return "Get_Version";
	case 5:
		return "Get_Serial_Number";
	case 6:
		return "Manufacturer";
	case 7:
		return "Echo_Loopback";
	default:
		return "Unknown";
	}
}

static char *facilitysel2str(uint16_t sel)
{
	switch (sel) {
	case 0x0000:
		return "Handset";
	case 0x0001:
		return "DTMF";
	case 0x0002:
		return "V.42 bis";
	case 0x0003:
		return "Supplementary Services";
	case 0x0004:
		return "Power management wakeup";
	case 0x0005:
		return "Line Interconnect";
	case 0x0006:
		return "DTMF";
	default:
		return "Unknown";
	}
}

static char *info2str(uint16_t info)
{
	switch (info) {
	case 0x0000:
		return "No error";
	case 0x0001:
		return "NCPI not supported by current protocol, NCPI ignored";
	case 0x0002:
		return "Flags not supported by current protocol, flags ignored";
	case 0x2001:
		return "Message not supported in current state";
	case 0x2002:
		return "Incorrect Controller/PLCI/NCCI";
	case 0x2003:
		return "No PLCI available";
	case 0x2004:
		return "No NCCI available";
	case 0x2005:
		return "No Listen resources available";
	case 0x2007:
		return "Illegal message parameter coding";
	case 0x2008:
		return "No interconnection resources available";
	case 0x3001:
		return "B1 protocol not supported";
	case 0x3002:
		return "B2 protocol not supported";
	case 0x3003:
		return "B3 protocol not supported";
	case 0x3004:
		return "B1 protocol parameter not supported";
	case 0x3005:
		return "B2 protocol parameter not supported";
	case 0x3006:
		return "B3 protocol parameter not supported";
	case 0x3007:
		return "B protocol combination not supported";
	case 0x3008:
		return "NCPI not supported";
	case 0x3009:
		return "CIP Value unknown";
	case 0x300A:
		return "Flags not supported (reserved bits)";
	case 0x300B:
		return "Facility not supported";
	case 0x300C:
		return "Data length not supported by current protocol";
	case 0x300D:
		return "Reset procedure not supported by current protocol";
	case 0x300F:
		return "Unsupported interoperability";
	case 0x3011:
		return "Facility specific function not supported";
	case 0x3301:
		return "Protocol error, Layer 1";
	case 0x3302:
		return "Protocol error, Layer 2";
	case 0x3303:
		return "Protocol error, Layer 3";
	case 0x3304:
		return "Another application got that call";
	case 0x3305:
		return "Cleared by Call Control Supervision";
	case 0x3400:
		/* The cause value received from the network in a cause
		 * information element (Octet 4) is indicated in the field 00 */
		return "Disconnect cause from the network in accordance with Q.850/ETS 300 102-1";
	default:
		return "Unknown";
	}
}

static void profile(int level, struct frame *frm)
{
	uint16_t nctr, nchn;
	uint32_t value;

	nctr = htons(get_u16(frm));
	nchn = htons(get_u16(frm));

	if (nchn > 0) {
		p_indent(level, frm);
		printf("Controller: %d\n", nctr);
		p_indent(level, frm);
		printf("Number of B-channels: %d\n", nchn);

		value = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("Global options: 0x%04x\n", value);
		value = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("B1 protocol support: 0x%08x\n", value);
		value = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("B2 protocol support: 0x%08x\n", value);
		value = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("B3 protocol support: 0x%08x\n", value);

		frm->ptr += 24;
		frm->len -= 24;

		p_indent(level, frm);
		printf("Manufacturer-specific information:\n");
		hex_dump(level, frm, 20);
	} else {
		p_indent(level, frm);
		printf("Number of controllers: %d\n", nctr);
	}
}

static void cmd_common(int level, uint8_t subcmd, struct frame *frm)
{
	uint32_t val;
	uint16_t info, ncci;
	uint8_t ctr, plci;

	val = htonl(get_u32(frm));
	ctr = val & 0xff;
	plci = (val & 0xff00) >> 8;
	ncci = (val & 0xffff0000) >> 16;

	p_indent(level, frm);
	printf("Controller: %d %s\n", ctr & 0x7f, ctr & 0x80 ? "Ext." : "Int.");

	if (plci > 0) {
		p_indent(level, frm);
		printf("PLCI: 0x%02x\n", plci);
	}

	if (ncci > 0) {
		p_indent(level, frm);
		printf("NCCI: 0x%04x\n", ncci);
	}

	if (subcmd == 0x81) {
		info = htons(get_u16(frm));
		p_indent(level, frm);
		printf("Info: 0x%04x (%s)\n", info, info2str(info));
	}
}

static void cmd_listen(int level, uint8_t subcmd, struct frame *frm)
{
	uint32_t mask;
	uint8_t len;

	cmd_common(level, subcmd, frm);

	if (subcmd == 0x80) {
		mask = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("Info mask: 0x%08x\n", mask);

		mask = htonl(get_u32(frm));
		p_indent(level, frm);
		printf("CIP mask:  0x%08x", mask);

		mask = htonl(get_u32(frm));
		if (mask > 0)
			printf(" 0x%08x\n", mask);
		else
			printf("\n");

		len = get_u8(frm);
		if (len > 0) {
			p_indent(level, frm);
			printf("Calling party number:\n");
			hex_dump(level, frm, len);
		}
		frm->ptr += len;
		frm->len -= len;

		len = get_u8(frm);
		if (len > 0) {
			p_indent(level, frm);
			printf("Calling party subaddress:\n");
			hex_dump(level, frm, len);
		}
		frm->ptr += len;
		frm->len -= len;
	}
}

static void cmd_info(int level, uint8_t subcmd, struct frame *frm)
{
	uint8_t len;
	uint16_t info;

	cmd_common(level, subcmd, frm);

	switch (subcmd) {
	case 0x80:
		len = get_u8(frm);
		if (len > 0) {
			p_indent(level, frm);
			printf("Called party number:\n");
			hex_dump(level, frm, len);
		}
		frm->ptr += len;
		frm->len -= len;

		len = get_u8(frm);
		if (len > 0) {
			p_indent(level, frm);
			printf("Additional info:\n");
			hex_dump(level, frm, len);
		}
		break;

	case 0x82:
		info = htons(get_u16(frm));
		p_indent(level, frm);
		printf("Info number: %d\n", info);

		len = get_u8(frm);
		if (len > 0) {
			p_indent(level, frm);
			printf("Info element:\n");
			hex_dump(level, frm, len);
		}
		break;
	}
}

static void cmd_interoperability(int level, uint8_t subcmd, struct frame *frm)
{
	uint16_t sel, func, info;
	uint16_t nconn, datablkcnt, datablklen;
	uint32_t ctr, value, major, minor;

	info = (subcmd == 0x81) ? htons(get_u16(frm)) : 0;
	sel = htons(get_u16(frm));
	get_u8(frm);
	if (subcmd != 0x83) {
		func = htons(get_u16(frm));
		get_u8(frm);
	} else
		func = 0;

	p_indent(level, frm);
	printf("Selector: 0x%04x (%s)\n", sel, interopsel2str(sel));

	switch (sel) {
	case 0x0001:
		p_indent(level, frm);
		printf("Function: %d (%s)\n", func, func2str(func));

		switch (subcmd) {
		case 0x80:
			switch (func) {
			case 0:
				nconn = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("maxLogicalConnections: %d\n", nconn);
				datablkcnt = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("maxBDataBlocks: %d\n", datablkcnt);
				datablklen = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("maxBDataLen: %d\n", datablklen);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				ctr = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Controller: %d\n", ctr);
				break;
			default:
				raw_dump(level + 1, frm);
				break;
			}
			break;

		case 0x81:
			switch (func) {
			case 0:
			case 1:
				info = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("Info: 0x%04x (%s)\n", info, info2str(info));
				break;
			case 2:
				info = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("Info: 0x%04x (%s)\n", info, info2str(info));
				get_u8(frm);
				profile(level + 1, frm);
				break;
			case 3:
				info = htons(get_u16(frm));
				p_indent(level + 1, frm);
				printf("Info: 0x%04x (%s)\n", info, info2str(info));
				ctr = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Controller: %d\n", ctr);
				get_u8(frm);
				p_indent(level + 1, frm);
				printf("Identification: \"%s\"\n", (char *) frm->ptr);
				break;
			case 4:
				value = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Return value: 0x%04x\n", value);
				ctr = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Controller: %d\n", ctr);
				p_indent(level + 1, frm);
				major = htonl(get_u32(frm));
				minor = htonl(get_u32(frm));
				printf("CAPI: %d.%d\n", major, minor);
				major = htonl(get_u32(frm));
				minor = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Manufacture: %u.%01x%01x-%02u (%d.%d)\n",
					(major & 0xf0) >> 4, (major & 0x0f) << 4,
					(minor & 0xf0) >> 4, minor & 0x0f,
					major, minor);
				break;
			case 5:
				value = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Return value: 0x%04x\n", value);
				ctr = htonl(get_u32(frm));
				p_indent(level + 1, frm);
				printf("Controller: %d\n", ctr);
				get_u8(frm);
				p_indent(level + 1, frm);
				printf("Serial number: %.7s\n", (char *) frm->ptr);
				break;
			default:
				raw_dump(level + 1, frm);
				break;
			}
			break;

		default:
			raw_dump(level, frm);
			break;
		}
		break;

	default:
		p_indent(level, frm);
		printf("Function: %d\n", func);
		if (subcmd == 0x81) {
			p_indent(level, frm);
			printf("Info: 0x%04x (%s)\n", info, info2str(info));
		}
		raw_dump(level + 1, frm);
		break;
	}
}

static void cmd_facility(int level, uint8_t subcmd, struct frame *frm)
{
	uint16_t sel;

	cmd_common(level, subcmd, frm);

	sel = htons(get_u16(frm));
	get_u8(frm);

	p_indent(level, frm);
	printf("Selector: 0x%04x (%s)\n", sel, facilitysel2str(sel));

	raw_dump(level, frm);
}

static void cmd_manufacturer(int level, uint8_t subcmd, struct frame *frm)
{
	uint32_t ctr, class, func;
	uint16_t len;
	unsigned char *id;

	ctr = htonl(get_u32(frm));
	p_indent(level, frm);
	printf("Controller: %d\n", ctr);

	id = (unsigned char *) frm->ptr;
	p_indent(level, frm);
	if (isprint(id[0]) && isprint(id[1]) && isprint(id[2]) && isprint(id[3]))
		printf("Manufacturer: %.4s", id);
	else
		printf("Manufacturer: 0x%02x 0x%02x 0x%02x 0x%02x",
						id[0], id[1], id[2], id[3]);
	frm->ptr += 4;
	frm->len -= 4;

	if (!strncmp(id, "AVM!", 4)) {
		class = htonl(get_u32(frm));
		func = htonl(get_u32(frm));
		len = get_u8(frm);
		if (len == 0xff)
			len = htons(get_u16(frm));

		printf(" [class %d func %d len %d]\n", class, func, len);
	} else
		printf("\n");

	raw_dump(level, frm);
}

void capi_dump(int level, struct frame *frm)
{
	uint16_t len, appl, msgnum;
	uint8_t cmd, subcmd;

	len = htons(get_u16(frm)) - 8;
	appl = htons(get_u16(frm));
	cmd = get_u8(frm);
	subcmd = get_u8(frm);
	msgnum = htons(get_u16(frm));

	p_indent(level, frm);

	printf("CAPI_%s_%s: appl %d msgnum %d len %d\n",
			cmd2str(cmd), subcmd2str(subcmd), appl, msgnum, len);

	switch (cmd) {
	case 0x05:
		cmd_listen(level + 1, subcmd, frm);
		break;
	case 0x08:
		cmd_info(level + 1, subcmd, frm);
		break;
	case 0x20:
		cmd_interoperability(level + 1, subcmd, frm);
		break;
	case 0x80:
		cmd_facility(level + 1, subcmd, frm);
		break;
	case 0xff:
		cmd_manufacturer(level + 1, subcmd, frm);
		break;
	default:
		raw_dump(level, frm);
		frm->ptr += len;
		frm->len -= len;
		break;
	}
}
