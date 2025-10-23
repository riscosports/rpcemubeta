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
#include <QDialog>
#include <QWidget>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>

#include "machinesettings.h"
#include "ui_machinesettings.h"

#include "rpcemu.h"

MachineSettings::MachineSettings(QWidget *parent) :
       QDialog(parent),
       ui(new Ui::MachineSettings)
{
    ui->setupUi(this);

    setWindowTitle("Configure machine");

    nat_list_dialog = new NatListDialog(this);

    // ******** MAIN TAB ********

    hardware_listwidget = new QListWidget();

    // Fill in ListWidget with models
    int modeliter =  0;
    while (modeliter < (int) Model_MAX) {
        QListWidgetItem *item = new QListWidgetItem(models[modeliter].name_gui);
        hardware_list_items.insert(hardware_list_items.end(), item);
        hardware_listwidget->addItem(item);
        modeliter++;
    }

    hardware_vbox = new QVBoxLayout();
    hardware_vbox->addWidget(hardware_listwidget);

    hardware_group_box = new QGroupBox("Hardware");
    hardware_group_box->setLayout(hardware_vbox);

    // Create Memory Group
    mem_4 = new QRadioButton("4 MB");
    mem_8 = new QRadioButton("8 MB");
    mem_16 = new QRadioButton("16 MB");
    mem_32 = new QRadioButton("32 MB (recommended)");
    mem_64 = new QRadioButton("64 MB (recommended)");
    mem_128 = new QRadioButton("128 MB (recommended)");
    mem_256 = new QRadioButton("256 MB");

    mem_group = new QButtonGroup();
    mem_group->addButton(mem_4);
    mem_group->addButton(mem_8);
    mem_group->addButton(mem_16);
    mem_group->addButton(mem_32);
    mem_group->addButton(mem_64);
    mem_group->addButton(mem_128);
    mem_group->addButton(mem_256);

    mem_vbox = new QVBoxLayout();
    mem_vbox->addWidget(mem_4);
    mem_vbox->addWidget(mem_8);
    mem_vbox->addWidget(mem_16);
    mem_vbox->addWidget(mem_32);
    mem_vbox->addWidget(mem_64);
    mem_vbox->addWidget(mem_128);
    mem_vbox->addWidget(mem_256);

    mem_group_box = new QGroupBox("RAM");
    mem_group_box->setLayout(mem_vbox);

    // Create VRAM group
    vram_0 = new QRadioButton("None");
    vram_2 = new QRadioButton("2 MB (8 MB if OS supported)");

    vram_group = new QButtonGroup();
    vram_group->addButton(vram_0);
    vram_group->addButton(vram_2);

    vram_vbox = new QVBoxLayout();
    vram_vbox->addWidget(vram_0);
    vram_vbox->addWidget(vram_2);

    vram_group_box = new QGroupBox("VRAM");
    vram_group_box->setLayout(vram_vbox);

    // Create refresh
    refresh_slider = new QSlider(Qt::Horizontal);
    refresh_slider->setRange(20, 100);
    refresh_slider->setTickPosition(QSlider::TicksBothSides);

    refresh_label = new QLabel("");

    refresh_hbox = new QHBoxLayout();
    refresh_hbox->addWidget(refresh_slider);
    refresh_hbox->addWidget(refresh_label);

    refresh_group_box = new QGroupBox("Video refresh rate");
    refresh_group_box->setLayout(refresh_hbox);

    // Create reduce cpu usage
    reduce_cpu = new QCheckBox("Reduce CPU usage");

    misc_hbox = new QHBoxLayout();
    misc_hbox->addWidget(reduce_cpu);
    misc_group_box = new QGroupBox("Misc Settings");
    misc_group_box->setLayout(misc_hbox);

    grid = new QGridLayout(this);
    grid->addWidget(hardware_group_box, 0, 0);
    grid->addWidget(mem_group_box, 0, 1);
    grid->addWidget(vram_group_box, 1, 0, 1, 2);    // span 2 columns
    grid->addWidget(refresh_group_box, 2, 0, 1, 2); // span 2 columns
    grid->addWidget(misc_group_box, 3, 0, 1, 2); // span 2 columns

    ui->main_tab->setLayout(grid);

    // ******** NETWORKING TAB ********

    // Create widgets and layout
    net_off = new QRadioButton("Off");
    net_nat = new QRadioButton("Network Address Translation (NAT)");
    net_bridging = new QRadioButton("Ethernet Bridging");
    net_tunnelling = new QRadioButton("IP Tunnelling");

    nat_conf_ports = new QPushButton("NAT Port Forwarding Rules...");
    nat_hbox = new QHBoxLayout();
    nat_hbox->insertSpacing(0, 48);
    nat_hbox->addWidget(nat_conf_ports);

    bridge_label = new QLabel("Bridge Name");
    bridge_name = new QLineEdit(QString("rpcemu"));
    bridge_name->setMinimumWidth(192);
    bridge_hbox = new QHBoxLayout();
    bridge_hbox->insertSpacing(0, 48);
    bridge_hbox->addWidget(bridge_label);
    bridge_hbox->addWidget(bridge_name);

    tunnelling_label = new QLabel("IP Address");
    tunnelling_name = new QLineEdit(QString("172.31.0.1"));
    tunnelling_name->setMinimumWidth(192);
    tunnelling_hbox = new QHBoxLayout();
    tunnelling_hbox->insertSpacing(0, 48);
    tunnelling_hbox->addWidget(tunnelling_label);
    tunnelling_hbox->addWidget(tunnelling_name);

    // Main layout
    vbox = new QVBoxLayout(this);
    vbox->addWidget(net_off);
    vbox->addWidget(net_nat);
    vbox->addLayout(nat_hbox);
    vbox->addWidget(net_bridging);
    vbox->addLayout(bridge_hbox);

    // IP Tunnelling is linux only
#if defined(Q_OS_LINUX)
    vbox->addWidget(net_tunnelling);
    vbox->addLayout(tunnelling_hbox);
#endif /* linux */

    ui->networking_tab->setLayout(vbox);

    // Connect actions to widgets
    connect(net_off, &QRadioButton::clicked, this, &MachineSettings::radio_clicked);
    connect(net_nat, &QRadioButton::clicked, this, &MachineSettings::radio_clicked);
    connect(net_bridging, &QRadioButton::clicked, this, &MachineSettings::radio_clicked);
    connect(net_tunnelling, &QRadioButton::clicked, this, &MachineSettings::radio_clicked);

    connect(nat_conf_ports, &QPushButton::clicked, this, &MachineSettings::nat_button_clicked);

    connect(this, &QDialog::accepted, this, &MachineSettings::dialog_accepted);
}

void
MachineSettings::nat_button_clicked()
{
    // modal
    nat_list_dialog->exec();

    config_save(&mach_config, mach_model);
}

void
MachineSettings::radio_clicked()
{
    if (net_nat->isChecked()) {
        nat_conf_ports->setEnabled(true);
    } else {
        nat_conf_ports->setEnabled(false);
    }

    if (net_bridging->isChecked()) {
        bridge_label->setEnabled(true);
        bridge_name->setEnabled(true);
    } else {
        bridge_label->setEnabled(false);
        bridge_name->setEnabled(false);
    }

    if (net_tunnelling->isChecked()) {
        tunnelling_label->setEnabled(true);
        tunnelling_name->setEnabled(true);
    } else {
        tunnelling_label->setEnabled(false);
        tunnelling_name->setEnabled(false);
    }
}
/**
 * Prepare the settings dialog for the machine the user is interested in
 * load the config and set the dialog items to the chosen values
 *
 * @param datadir path to machine location with rpc.cfg in
 */
void
MachineSettings::InitialiseMachine(const char *datadir)
{
    // Always open the first tab
    ui->tabWidget->setCurrentIndex(0);

    rpcemu_set_datadir(datadir);
    config_load(&mach_config, &mach_model);

    // ******** Main Tab ********

    // Hardware Model
    hardware_listwidget->setCurrentItem(hardware_list_items.at(static_cast<unsigned int>(mach_model)));

    // RAM Size
    mem_4->setChecked(false);
    mem_8->setChecked(false);
    mem_16->setChecked(false);
    mem_32->setChecked(false);
    mem_64->setChecked(false);
    mem_128->setChecked(false);
    mem_256->setChecked(false);

    switch(mach_config.mem_size) {
    case   4: mem_4->setChecked(true);   break;
    case   8: mem_8->setChecked(true);   break;
    case  16: mem_16->setChecked(true);  break;
    case  32: mem_32->setChecked(true);  break;
    case  64: mem_64->setChecked(true);  break;
    case 128: mem_128->setChecked(true); break;
    case 256: mem_256->setChecked(true); break;
    default: error("configuredialog.cpp: unhandled memsize %u", mach_config.mem_size); exit(EXIT_FAILURE);
    }

    // VRAM
    vram_0->setChecked(false);
    vram_2->setChecked(false);

    switch (mach_config.vram_size) {
    case 0:
        vram_0->setChecked(true);
        break;
    default:
        vram_2->setChecked(true);
        break;
    }

    // Video Refresh Rate
    refresh_slider->setValue(mach_config.refresh);
    refresh_label->setText(QString::number(mach_config.refresh) + " Hz");

    // Misc settings
    reduce_cpu->setChecked(mach_config.cpu_idle);

    // ******** Networking Tab ********

    // Select the correct radio button
    net_off->setChecked(false);
    net_nat->setChecked(false);
    net_bridging->setChecked(false);
    net_tunnelling->setChecked(false);
    switch (mach_config.network_type) {
    case NetworkType_Off:
        net_off->setChecked(true);
        break;
    case NetworkType_NAT:
        net_nat->setChecked(true);
        break;
    case NetworkType_EthernetBridging:
        net_bridging->setChecked(true);
        break;
    case NetworkType_IPTunnelling:
        net_tunnelling->setChecked(true);
        break;
    }

    // Use the helper function to grey out the boxes of unselected
    // network types
    radio_clicked();

    if(mach_config.bridgename && mach_config.bridgename[0] != '\0') {
        bridge_name->setText(mach_config.bridgename);
    } else {
        bridge_name->setText("");
    }

    if(mach_config.ipaddress && mach_config.ipaddress[0] != '\0') {
        tunnelling_name->setText(mach_config.ipaddress);
    } else {
        tunnelling_name->setText("");
    }
}

/**
 * User clicked OK on the Configure dialog box
 */
void
MachineSettings::dialog_accepted()
{
    char *bridgename, *ipaddress;

    // ******** MAIN TAB ********
    mach_model = static_cast<Model>(hardware_listwidget->currentRow());

    // RAM Size
    if (mem_4->isChecked())   mach_config.mem_size =   4;
    if (mem_8->isChecked())   mach_config.mem_size =   8;
    if (mem_16->isChecked())  mach_config.mem_size =  16;
    if (mem_32->isChecked())  mach_config.mem_size =  32;
    if (mem_64->isChecked())  mach_config.mem_size =  64;
    if (mem_128->isChecked()) mach_config.mem_size = 128;
    if (mem_256->isChecked()) mach_config.mem_size = 256;

    // VRAM
    if (vram_0->isChecked()) mach_config.vram_size = 0;
    if (vram_2->isChecked()) mach_config.vram_size = 8;

    // Video Refresh Rate
    mach_config.refresh = refresh_slider->value();

    // Misc Settings
    if (reduce_cpu->isChecked()) {
        mach_config.cpu_idle = 1;
    } else {
        mach_config.cpu_idle = 0;
    }

    // ******** Networking Tab ********

    // Fill in the choices from the dialog box
    if (net_off->isChecked()) {
        mach_config.network_type = NetworkType_Off;
    } else if (net_nat->isChecked()) {
        mach_config.network_type = NetworkType_NAT;
    } else if (net_bridging->isChecked()) {
        mach_config.network_type = NetworkType_EthernetBridging;
    } else if (net_tunnelling->isChecked()) {
        mach_config.network_type = NetworkType_IPTunnelling;
    }

    bridgename = bridge_name->text().toUtf8().data();
    ipaddress = tunnelling_name->text().toUtf8().data();

    if (mach_config.bridgename == nullptr) {
        mach_config.bridgename = strdup(bridgename);
    } else if (strcmp(mach_config.bridgename, bridgename) != 0) {
        free(mach_config.bridgename);
        mach_config.bridgename = strdup(bridgename);
    }
    if (mach_config.ipaddress == nullptr) {
        mach_config.ipaddress = strdup(ipaddress);
    } else if (strcmp(mach_config.ipaddress, ipaddress) != 0) {
        free(mach_config.ipaddress);
        mach_config.ipaddress = strdup(ipaddress);
    }

    config_save(&mach_config, mach_model);
}
