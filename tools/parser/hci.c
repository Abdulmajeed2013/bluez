/*
 *
 *  Bluetooth packet analyzer - HCI parser
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2004  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  $Id$
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "parser.h"

static uint16_t manufacturer = DEFAULT_COMPID;

static inline uint16_t get_manufacturer(void)
{
	return (manufacturer == DEFAULT_COMPID ? parser.defcompid : manufacturer);
}

static char *event_map[] = {
	"Unknown",
	"Inquiry Complete",
	"Inquiry Result",
	"Connect Complete",
	"Connect Request",
	"Disconn Complete",
	"Auth Complete",
	"Remote Name Req Complete",
	"Encrypt Change",
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
	"Page Scan Repetition Mode Change",
	"Flow Specification Complete",
	"Inquiry Result with RSSI",
	"Read Remote Extended Features",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Synchronous Connect Complete",
	"Synchronous Connect Changed"
};
#define EVENT_NUM 43

static char *cmd_linkctl_map[] = {
	"Unknown",
	"Inquiry",
	"Inquiry Cancel",
	"Periodic Inquiry Mode",
	"Exit Periodic Inquiry Mode",
	"Create Connection",
	"Disconnect",
	"Add SCO Connection",
	"Create Connection Cancel",
	"Accept Connection Request",
	"Reject Connection Request",
	"Link Key Request Reply",
	"Link Key Request Negative Reply",
	"PIN Code Request Reply",
	"PIN Code Request Negative Reply",
	"Change Connection Packet Type",
	"Unknown",
	"Authentication Requested",
	"Unknown",
	"Set Connection Encryption",
	"Unknown",
	"Change Connection Link Key",
	"Unknown",
	"Master Link Key",
	"Unknown",
	"Remote Name Request",
	"Remote Name Request Cancel",
	"Read Remote Supported Features",
	"Read Remote Extended Features",
	"Read Remote Version Information",
	"Unknown",
	"Read Clock Offset",
	"Read LMP Handle"
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Setup Synchronous Connection",
	"Accept Synchronous Connection",
	"Reject Synchronous Connection"
};
#define CMD_LINKCTL_NUM 42

static char *cmd_linkpol_map[] = {
	"Unknown",
	"Hold Mode",
	"Unknown",
	"Sniff Mode",
	"Exit Sniff Mode",
	"Park State",
	"Exit Park State",
	"QoS Setup",
	"Unknown",
	"Role Discovery",
	"Unknown",
	"Switch Role",
	"Read Link Policy Settings",
	"Write Link Policy Settings",
	"Read Default Link Policy Settings",
	"Write Default Link Policy Settings",
	"Flow Specification"
};
#define CMD_LINKPOL_NUM 16

static char *cmd_hostctl_map[] = {
	"Unknown",
	"Set Event Mask",
	"Unknown",
	"Reset",
	"Unknown",
	"Set Event Filter",
	"Unknown",
	"Unknown",
	"Flush",
	"Read PIN Type ",
	"Write PIN Type",
	"Create New Unit Key",
	"Unknown",
	"Read Stored Link Key",
	"Unknown",
	"Unknown",
	"Unknown",
	"Write Stored Link Key",
	"Delete Stored Link Key",
	"Write Local Name",
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
	"Read Synchronous Flow Control Enable",
	"Write Synchronous Flow Control Enable",
	"Unknown",
	"Set Host Controller To Host Flow Control",
	"Unknown",
	"Host Buffer Size",
	"Unknown",
	"Host Number of Completed Packets",
	"Read Link Supervision Timeout",
	"Write Link Supervision Timeout",
	"Read Number of Supported IAC",
	"Read Current IAC LAP",
	"Write Current IAC LAP",
	"Read Page Scan Period Mode",
	"Write Page Scan Period Mode",
	"Read Page Scan Mode",
	"Write Page Scan Mode",
	"Set AFH Host Channel Classification",
	"Unknown",
	"Unknown",
	"Read Inquiry Scan Type",
	"Write Inquiry Scan Type",
	"Read Inquiry Mode",
	"Write Inquiry Mode",
	"Read Page Scan Type",
	"Write Page Scan Type",
	"Read AFH Channel Assessment Mode",
	"Write AFH Channel Assessment Mode"
};
#define CMD_HOSTCTL_NUM 73

static char *cmd_info_map[] = {
	"Unknown",
	"Read Local Version Information",
	"Read Local Supported Commands",
	"Read Local Supported Features",
	"Read Local Extended Features",
	"Read Buffer Size",
	"Unknown",
	"Read Country Code",
	"Unknown",
	"Read BD ADDR"
};
#define CMD_INFO_NUM 9

static char *cmd_status_map[] = {
	"Unknown",
	"Read Failed Contact Counter",
	"Reset Failed Contact Counter",
	"Read Link Quality",
	"Unknown",
	"Read RSSI",
	"Read AFH Channel Map",
	"Read Clock"
};
#define CMD_STATUS_NUM 7

static inline void command_dump(int level, struct frame *frm)
{
	hci_command_hdr *hdr = frm->ptr;
	uint16_t opcode = btohs(hdr->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	char *cmd;

	if (p_filter(FILT_HCI))
		return;

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

	case OGF_TESTING_CMD:
		cmd = "Testing";
		break;

	case OGF_VENDOR_CMD:
		cmd = "Vendor";
		break;

	default:
		cmd = "Unknown";
		break;
	}

	p_indent(level, frm);

	printf("HCI Command: %s (0x%2.2x|0x%4.4x) plen %d\n", 
		cmd, ogf, ocf, hdr->plen);

	frm->ptr += HCI_COMMAND_HDR_SIZE;
	frm->len -= HCI_COMMAND_HDR_SIZE;

	if (ogf == OGF_VENDOR_CMD && ocf == 0 && get_manufacturer() == 10) {
		csr_dump(level + 1, frm);
		return;
	}

	raw_dump(level, frm);
}

static inline void event_dump(int level, struct frame *frm)
{
	hci_event_hdr *hdr = frm->ptr;
	uint8_t event = hdr->evt;

	if (p_filter(FILT_HCI))
		return;

	p_indent(level, frm);

	if (hdr->evt <= EVENT_NUM) {
		printf("HCI Event: %s (0x%2.2x) plen %d\n",
			event_map[hdr->evt], hdr->evt, hdr->plen);
	} else if (hdr->evt == EVT_TESTING) {
		printf("HCI Event: Testing (0x%2.2x) plen %d\n", hdr->evt, hdr->plen);
	} else if (hdr->evt == EVT_VENDOR) {
		printf("HCI Event: Vendor (0x%2.2x) plen %d\n", hdr->evt, hdr->plen);
		if (get_manufacturer() == 10) {
			frm->ptr += HCI_EVENT_HDR_SIZE;
			frm->len -= HCI_EVENT_HDR_SIZE;
			csr_dump(level + 1, frm);
			return;
		}
	} else
		printf("HCI Event: code 0x%2.2x plen %d\n", hdr->evt, hdr->plen);

	frm->ptr += HCI_EVENT_HDR_SIZE;
	frm->len -= HCI_EVENT_HDR_SIZE;

	if (event == EVT_CMD_COMPLETE) {
		evt_cmd_complete *cc = frm->ptr;
		if (cc->opcode == cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION)) {
			read_local_version_rp *rp = frm->ptr + EVT_CMD_COMPLETE_SIZE;
			manufacturer = rp->manufacturer;
		}
	}

	raw_dump(level, frm);
}

static inline void acl_dump(int level, struct frame *frm)
{
	hci_acl_hdr *hdr = (void *) frm->ptr;
	uint16_t handle = btohs(hdr->handle);
	uint16_t dlen = btohs(hdr->dlen);
	uint8_t flags = acl_flags(handle);

	if (!p_filter(FILT_HCI)) {
		p_indent(level, frm);
		printf("ACL data: handle 0x%4.4x flags 0x%2.2x dlen %d\n",
			acl_handle(handle), flags, dlen);
		level++;
	}

	frm->ptr += HCI_ACL_HDR_SIZE;
	frm->len -= HCI_ACL_HDR_SIZE;
	frm->flags  = flags;
	frm->handle = acl_handle(handle);

	if (parser.filter & ~FILT_HCI)
		l2cap_dump(level, frm);
	else
		raw_dump(level, frm);
}

static inline void sco_dump(int level, struct frame *frm)
{
	hci_sco_hdr *hdr = (void *) frm->ptr;
	uint16_t handle = btohs(hdr->handle);

	if (!p_filter(FILT_SCO)) {
		p_indent(level, frm);
		printf("SCO data: handle 0x%4.4x dlen %d\n",
			acl_handle(handle), hdr->dlen);
		level++;

		frm->ptr += HCI_SCO_HDR_SIZE;
		frm->len -= HCI_SCO_HDR_SIZE;
		raw_dump(level, frm);
	}
}

static inline void vendor_dump(int level, struct frame *frm)
{
	p_indent(level, frm);
	printf("Vendor data: len %d\n", frm->len);
	raw_dump(level, frm);
}

void hci_dump(int level, struct frame *frm)
{
	uint8_t type = *(uint8_t *)frm->ptr; 

	frm->ptr++; frm->len--;

	switch (type) {
	case HCI_COMMAND_PKT:
		command_dump(level, frm);
		break;

	case HCI_EVENT_PKT:
		event_dump(level, frm);
		break;

	case HCI_ACLDATA_PKT:
		acl_dump(level, frm);
		break;

	case HCI_SCODATA_PKT:
		sco_dump(level, frm);
		break;

	case HCI_VENDOR_PKT:
		vendor_dump(level, frm);
		break;

	default:
		if (p_filter(FILT_HCI))
			break;

		p_indent(level, frm);
		printf("Unknown: type 0x%2.2x len %d\n", type, frm->len);
		raw_dump(level, frm);
		break;
	}
}
