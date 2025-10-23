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
#ifndef MAIN_H
#define MAIN_H


#include <QApplication>

#include "mainwindow.h"


typedef struct {
    QString name;
    QString directory;
    bool isLocalInstall; // Local install means the install next to the main rpcemu-interpretter/recompiler binaries, it should be editable but not deletable
} MachineInstance;

extern std::vector<MachineInstance> machine_instances;

bool config_machine_new(QString name, QString directory);
bool config_machine_import(QString name, QString directory, bool isLocalInstall);
bool config_machine_remove(QString name, QString directory);

#endif // MAIN_H
