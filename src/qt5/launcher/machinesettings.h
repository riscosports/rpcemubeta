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
#ifndef MACHINESETTINGS_H
#define MACHINESETTINGS_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

#include "rpcemu.h"
#include "lnat_list_dialog.h"

namespace Ui {
class MachineSettings;
}

class MachineSettings : public QDialog
{
public:
    MachineSettings(QWidget *parent = nullptr);
    void InitialiseMachine(const char *datadir);

private slots:
    void radio_clicked();
    void dialog_accepted();
    void nat_button_clicked();

private:
    Config mach_config;
    Model mach_model;

    Ui::MachineSettings *ui;

    NatListDialog *nat_list_dialog;

    // Main Tab
    QGridLayout *grid;
    QListWidget *hardware_listwidget;
    std::vector<QListWidgetItem *>  hardware_list_items;
    QVBoxLayout *hardware_vbox;
    QGroupBox *hardware_group_box;

    QButtonGroup *mem_group;
    QRadioButton *mem_4, *mem_8, *mem_16, *mem_32, *mem_64, *mem_128, *mem_256;
    QVBoxLayout *mem_vbox;
    QGroupBox *mem_group_box;

    QButtonGroup *vram_group;
    QRadioButton *vram_0, *vram_2;
    QVBoxLayout *vram_vbox;
    QGroupBox *vram_group_box;

    QSlider *refresh_slider;
    QLabel *refresh_label;
    QHBoxLayout *refresh_hbox;
    QGroupBox *refresh_group_box;

    QGroupBox *misc_group_box;
    QHBoxLayout *misc_hbox;
    QCheckBox *reduce_cpu;

    // Networking Tab
    QRadioButton *net_off;
    QRadioButton *net_nat;
    QRadioButton *net_bridging;
    QRadioButton *net_tunnelling;

    QPushButton *nat_conf_ports;
    QHBoxLayout * nat_hbox;

    QLabel *bridge_label;
    QLineEdit *bridge_name;
    QHBoxLayout *bridge_hbox;

    QLabel *tunnelling_label;
    QLineEdit *tunnelling_name;
    QHBoxLayout *tunnelling_hbox;

    QVBoxLayout *vbox;

};

#endif // MACHINESETTINGS_H
