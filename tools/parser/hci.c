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

#include <sys/socket.h>
#include <sys/types.h>
#include <asm/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "parser.h"

char *event_map[] = {
	"Unknown",
	"Inquiry Complete",
	"Inquiry Result",
	"Connect Complete",
	"Connect Request",
	"Disconn Complete",
	"Auth Complete",
	"Remote Name Req Complete",
	"Encryp Change",
	"Change Connection Link Key Complete",
	"Master Link Key Complete",
	"Read Remote Supported Features",
	"Read Remote Ver Info Complete",
	"QoS Setup Complete",
	"Command Complete",
	"Command Status",
	"Hardware Error",
	"Flush Occurred",
	"Role Change",
	"Number of Completed Packets",
	"Mode Change",
	"Return Link Keys",
	"PIN Code Request",
	"Link Key Request",
	"Link Key Notification",
	"Loopback Command",
	"Data Buffer Overflow",
	"Max Slots Change",
	"Read Clock Offset Complete",
	"Connection Packet Type Changed",
	"QoS Violation",
	"Page Scan Mode Change",
	"Page Scan Repetition Mode Change"
};
#define EVENT_NUM	32

char *cmd_linkctl_map[] = {
	"Unknown",
	"Inquiry",
	"Inquiry Cancel",
	"Periodic Inquiry Mode",
	"Exit Periodic Inquiry Mode",
	"Create Connection",
	"Disconnect",
	"Add SCO Connection",
	"Unknown",
	"Accept Connection Request",
	"Reject Connection Request",
	"Link Key Request Reply",
	"Link Key Request Negative Reply",
	"PIN Code Request Reply",
	"PIN Code Request Negative Reply",
	"Change Connection Packet Type",
	"Authentication Requested",
	"Set Connection Encryption",
	"Change Connection Link Key",
	"Master Link Key",
	"Remote Name Request",
	"Read Remote Supported Features",
	"Read Remote Version Information",
	"Read Clock offset"
};
#define CMD_LINKCTL_NUM	23

char *cmd_linkpol_map[] = {
	"Unknown",
	"Hold Mode",
	"Sniff Mode",
	"Exit Sniff Mode",
	"Park Mode",
	"Exit Park Mode",
	"QoS Setup",
	"Role Discovery",
	"Switch Role",
	"Read Link Policy Settings",
	"Write Link Policy Settings"
};
#define CMD_LINKPOL_NUM 10

char *cmd_hostctl_map[] = {
	"Unknown",
	"Hold Mode",
	"Sniff Mode",
	"Exit Sniff Mode",
	"Set Event Mask",
	"Reset",
	"Set Event Filter",
	"Flush",
	"Read PIN Type ",
	"Write PIN Type",
	"Create New Unit Key",
	"Read Stored Link Key",
	"Write Stored Link Key",
	"Delete Stored Link Key",
	"Change Local Name",
	"Read Local Name",
	"Read Connection Accept Timeout",
	"Write Connection Accept Timeout",
	"Read Page Timeout",
	"Write Page Timeout",
	"Read Scan Enable",
	"Write Scan Enable",
	"Read Page Scan Activity",
	"Write Page Scan Activity",
	"Read Inquiry Scan Activity",
	"Write Inquiry Scan Activity",
	"Read Authentication Enable",
	"Write Authentication Enable",
	"Read Encryption Mode",
	"Write Encryption Mode",
	"Read Class of Device",
	"Write Class of Device",
	"Read Voice Setting",
	"Write Voice Setting",
	"Read Automatic Flush Timeout",
	"Write Automatic Flush Timeout",
	"Read Num Broadcast Retransmissions",
	"Write Num Broadcast Retransmissions",
	"Read Hold Mode Activity ",
	"Write Hold Mode Activity",
	"Read Transmit Power Level",
	"Read SCO Flow Control Enable",
	"Write SCO Flow Control Enable",
	"Set Host Controller To Host Flow Control",
	"Host Buffer Size",
	"Host Number of Completed Packets",
	"Read Link Supervision Timeout",
	"Write Link Supervision Timeout",
	"Read Number of Supported IAC",
	"Read Current IAC LAP",
	"Write Current IAC LAP",
	"Read Page Scan Period Mode",
	"Write Page Scan Period Mode",
	"Read Page Scan Mode",
	"Write Page Scan Mode"
};
#define CMD_HOSTCTL_NUM 51

char *cmd_info_map[] = {
	"Unknown",
	"Read Local Version Information",
	"Read Local Supported Features",
	"Read Buffer Size",
	"Read Country Code",
	"Read BD ADDR"
};
#define CMD_INFO_NUM 5

char *cmd_status_map[] = {
	"Unknown",
	"Read Failed Contact Counter",
	"Reset Failed Contact Counter",
	"Get Link Quality",
	"Read RSSI"
};
#define CMD_STATUS_NUM 5

static inline void command_dump(void *ptr, int len)
{
	hci_command_hdr *hdr = ptr;
	__u16 opcode = __le16_to_cpu(hdr->opcode);
	__u16 ogf = cmd_opcode_ogf(opcode);
	__u16 ocf = cmd_opcode_ocf(opcode);
	char *cmd;

	ptr += HCI_COMMAND_HDR_SIZE;
	len -= HCI_COMMAND_HDR_SIZE;

	switch (ogf) {
	case OGF_INFO_PARAM:
		if (ocf <= CMD_INFO_NUM)
			cmd = cmd_info_map[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_HOST_CTL:
		if (ocf <= CMD_HOSTCTL_NUM)
			cmd = cmd_hostctl_map[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_LINK_CTL:
		if (ocf <= CMD_LINKCTL_NUM)
			cmd = cmd_linkctl_map[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_LINK_POLICY:
		if (ocf <= CMD_LINKPOL_NUM)
			cmd = cmd_linkpol_map[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_STATUS_PARAM:
		if (ocf <= CMD_STATUS_NUM)
			cmd = cmd_status_map[ocf];
		else
			cmd = "Unknown";
		break;

	default:
		cmd = "Unknown";
		break;
	}

	printf("HCI Command: %s(0x%2.2x|0x%4.4x) plen %d\n", cmd, ogf, ocf, hdr->plen);
	raw_dump(1, ptr, len);
}

static inline void event_dump(void *ptr, int len)
{
	hci_event_hdr *hdr = ptr;
	
	ptr += HCI_EVENT_HDR_SIZE;
	len -= HCI_EVENT_HDR_SIZE;

	if (hdr->evt <= EVENT_NUM)
		printf("HCI Event: %s(0x%2.2x) plen %d\n",
			event_map[hdr->evt], hdr->evt, hdr->plen);
	else
		printf("HCI Event: code 0x%2.2x plen %d\n", hdr->evt, hdr->plen);
	raw_dump(1, ptr, len);
}

static inline void acl_dump(void *ptr, int len)
{
	hci_acl_hdr *hdr = ptr;
	__u16 handle = __le16_to_cpu(hdr->handle);
	__u16 dlen = __le16_to_cpu(hdr->dlen);
	__u8 flags = acl_flags(handle);
	
	printf("ACL data: handle 0x%4.4x flags 0x%2.2x dlen %d\n",
		acl_handle(handle), flags, dlen);
	
	ptr += HCI_ACL_HDR_SIZE;
	len -= HCI_ACL_HDR_SIZE;

	if (flags & ACL_START) {
		l2cap_dump(1, ptr, len, flags);
	} else {
		raw_dump(1, ptr, len);
	}
}

void hci_dump(int level, __u8 *data, int len)
{
	unsigned char *ptr = data;
	__u8 type;

	type = *ptr++; len--;
	
	switch (type) {
	case HCI_COMMAND_PKT:
		command_dump(ptr, len);
		break;

	case HCI_EVENT_PKT:
		event_dump(ptr, len);
		break;

	case HCI_ACLDATA_PKT:
		acl_dump(ptr, len);
		break;

	default:
		printf("Unknown: type 0x%2.2x len %d\n", type, len);
		raw_dump(1, ptr, len);
		break;
	}
}
