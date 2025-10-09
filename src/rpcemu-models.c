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
#include "rpcemu.h"
#include "cmos.h"

/** Array of details of models the emulator can emulate, must be kept in sync with
    Model enum in rpcemu.h */
const Model_Details models[] = {
	{ "Risc PC - ARM610",                "RPC610", CPUModel_ARM610,    IOMDType_IOMD,      SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "Risc PC - ARM710",                "RPC710", CPUModel_ARM710,    IOMDType_IOMD,      SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "Risc PC - StrongARM",             "RPCSA",  CPUModel_SA110,     IOMDType_IOMD,      SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "A7000",                           "A7000",  CPUModel_ARM7500,   IOMDType_ARM7500,   SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "A7000+ (experimental)",           "A7000+", CPUModel_ARM7500FE, IOMDType_ARM7500FE, SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "Risc PC - ARM810 (experimental)", "RPC810", CPUModel_ARM810,    IOMDType_IOMD,      SuperIOType_FDC37C665GT, I2C_PCF8583 },
	{ "Phoebe (RPC2)",                   "Phoebe", CPUModel_SA110,     IOMDType_IOMD2,     SuperIOType_FDC37C672,   I2C_PCF8583 | I2C_SPD_DIMM0 }
};
