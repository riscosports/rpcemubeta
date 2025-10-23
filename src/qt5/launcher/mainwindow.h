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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "about_dialog.h"
#include "machinenew.h"
#include "machineimport.h"
#include "machinesettings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void add_machine_to_list(QString name, bool front);
    void remove_machine_from_list(QString name);

	// Paths to other RPCEmu binaries
    QString pathInterpreter;
    QString pathRecompiler;

private slots:
    // Menus
    void menu_online_manual();
    void menu_visit_website();
    void menu_about();

    // Toolbar
    void new_triggered();
    void import_triggered();
    void settings_triggered();
    void remove_triggered();
    void startInterpreter_triggered();
    void startRecompiler_triggered();
    void openFolder_triggered();

    void machine_list_item_selection_changed();

private:
    Ui::MainWindow *ui;

    int machine_list_current_row; ///< Which row in the machine list is currently selected (or -1 if none)

    // Dialogs
    AboutDialog *about_dialog;
    MachineNew *machinenew_dialog;
    MachineImport *machineimport_dialog;
    MachineSettings *machinesettings_dialog;
};

#endif // MAINWINDOW_H
