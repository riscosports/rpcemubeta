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
#ifndef RPCEMU_NAT_RULES_H
#define RPCEMU_NAT_RULES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	PORT_FORWARD_NONE = 0,		///< No valid rule stored
	PORT_FORWARD_TCP  = 1,		///< A TCP rule
	PORT_FORWARD_UDP  = 2,		///< A UDP rule
	// All other values reserved
} PortForwardType;

typedef struct {
	PortForwardType	type;		///< Which type of rule to use, or NONE for no rule
	uint16_t	emu_port;	///< Port to connect to on the emulated machine
	uint16_t	host_port;	///< Port to connect to on the host machine
} PortForwardRule;

#define MAX_PORT_FORWARDS 32

extern PortForwardRule port_forward_rules[MAX_PORT_FORWARDS]; ///< Port forward rules accross the NAT

extern void rpcemu_nat_forward_add(PortForwardRule rule);
extern void rpcemu_nat_forward_remove(PortForwardRule rule);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif