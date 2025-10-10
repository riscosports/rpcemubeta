/*
  RPCEmu - An Acorn system emulator

  Copyright (C) 2021 Peter Howkins

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Functions to manipulate the list of Natwork address translation's port-forwarding rules */

#include <assert.h>

#include "rpcemu.h"
#include "rpcemu-nat-rules.h"

PortForwardRule port_forward_rules[MAX_PORT_FORWARDS]; ///< Port forward rules accross the NAT


/**
 * Add a forwarding rule to the NAT
 *
 * @param type      TCP or UDP
 * @param emu_port  port number on emulated machine
 * @param host_port port number on host machine
 */
void
rpcemu_nat_forward_add(PortForwardRule rule)
{
	int i;

	rpclog("Config: Adding NAT forwarding rule %d %u %u\n", rule.type, rule.emu_port, rule.host_port);

	// Detect duplicate rules
	for (i = 0; i < MAX_PORT_FORWARDS; i++) {
		if (port_forward_rules[i].type == rule.type
		    && port_forward_rules[i].emu_port == rule.emu_port)
		{
			rpclog("Config: Discarding duplicate NAT forwarding rule for type %d emu_port %u\n",
			    rule.type, rule.emu_port);
			return;
		}
		if (port_forward_rules[i].type == rule.type
		    && port_forward_rules[i].host_port == rule.host_port)
		{
			rpclog("Config: Discarding duplicate NAT forwarding rule for type %d host_port %u\n",
			    rule.type, rule.host_port);
			return;
		}
	}

	// Find an empty slot and fill it in
	for (i = 0; i < MAX_PORT_FORWARDS; i++) {
		if (port_forward_rules[i].type == PORT_FORWARD_NONE) {
			port_forward_rules[i] = rule;
			return;
		}
	}

	// No slot found for rule
	rpclog("Config: Ran out of space for NAT port forward rules\n");
}

/**
 * Remove a forwarding rule in the NAT
 *
 * @param type      TCP or UDP
 * @param emu_port  port number on emulated machine
 * @param host_port port number on host machine
 */
void
rpcemu_nat_forward_remove(PortForwardRule rule)
{
	int i;

	for (i = 0; i < MAX_PORT_FORWARDS; i++) {
		if (port_forward_rules[i].type == rule.type
		    && port_forward_rules[i].emu_port == rule.emu_port
		    && port_forward_rules[i].host_port == rule.host_port)
		{
			port_forward_rules[i].type      = PORT_FORWARD_NONE;
			port_forward_rules[i].emu_port  = 0;
			port_forward_rules[i].host_port = 0;

			return;
		}
	}

	// rule not found, should be impossible
	assert(0);
}