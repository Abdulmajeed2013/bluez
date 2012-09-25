/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Nokia Corporation
 *  Copyright (C) 2012  Marcel Holtmann <marcel@holtmann.org>
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>

#include "manager.h"
#include "adapter.h"
#include "device.h"
#include "profile.h"
#include "server.h"

struct btd_profile time_profile = {
	.name		= "gatt-time-server",
	.adapter_probe	= time_server_init,
	.adapter_remove	= time_server_exit,
};

int time_manager_init(void)
{
	btd_profile_register(&time_profile);

	return 0;
}

void time_manager_exit(void)
{
	btd_profile_unregister(&time_profile);
}
