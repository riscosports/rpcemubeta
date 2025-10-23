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
#include <iostream>
#include <vector>
#include <cassert>

#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QProcess>
#include <QUrl>
#include <QMessageBox>

#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "machinenew.h"
#include "machineimport.h"
#include "machinesettings.h"

#include "about_dialog.h"
#include "rpcemu.h"


// TODO duplicated, refactor
#define URL_MANUAL     "http://www.marutan.net/rpcemu/manual/"
#define URL_WEBSITE    "http://www.marutan.net/rpcemu/"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("RPCEmu v" VERSION);

    ui->overview_groupBox->setVisible(false);

    // Actions on File menu
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Actions on About menu
    connect(ui->actionOnline_Manual, &QAction::triggered, this, &MainWindow::menu_online_manual);
    connect(ui->actionVisit_Website, &QAction::triggered, this, &MainWindow::menu_visit_website);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::menu_about);

    // Window Widget connections
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::new_triggered);
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::import_triggered);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::settings_triggered);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::remove_triggered);
    connect(ui->actionStartInterpreter, &QAction::triggered, this, &MainWindow::startInterpreter_triggered);
    connect(ui->actionStartRecompiler, &QAction::triggered, this, &MainWindow::startRecompiler_triggered);
    connect(ui->machine_list, &QListWidget::itemSelectionChanged, this, &MainWindow::machine_list_item_selection_changed);

    // Widgets on the machine overview pane
    connect(ui->pushButton_openFolder, &QAbstractButton::clicked, this, &MainWindow::openFolder_triggered);

    // Create sub dialogs
    about_dialog = new AboutDialog(this);
    machinesettings_dialog = new MachineSettings(this);
    machinenew_dialog = new MachineNew(this);
    machineimport_dialog = new MachineImport(this);

    // Fill in the machine list with the ones from the config file
    for (MachineInstance machine_instance: machine_instances) {
        add_machine_to_list(machine_instance.name, false);
    }
    machine_list_current_row = -1; // -1 = no selected row
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::menu_online_manual()
{
    QDesktopServices::openUrl(QUrl(URL_MANUAL));
}

void
MainWindow::menu_visit_website()
{
    QDesktopServices::openUrl(QUrl(URL_WEBSITE));
}

/**
 * Someone clicked the about item on the help menu
 */
void
MainWindow::menu_about()
{
    about_dialog->show(); // Modeless
}

/**
 * Item within the machine list has changed
 */
void
MainWindow::machine_list_item_selection_changed()
{
    machine_list_current_row = ui->machine_list->currentRow();

    if(machine_list_current_row == -1
       || static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()) // We can get called when the machines
                                                                           // array has been shrunk, but the gui
                                                                           // machine list hasn't been so far.
    {
         // Unselected the item in the machine list
         ui->actionSettings->setEnabled(false);
         ui->actionRemove->setEnabled(false);
         ui->actionStartInterpreter->setEnabled(false);
         ui->actionStartRecompiler->setEnabled(false);
         ui->overview_groupBox->setVisible(false);
    } else {
        // Selected an item in the machine list
        ui->actionSettings->setEnabled(true);
        if(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).isLocalInstall) {
            ui->actionRemove->setEnabled(false);
        } else {
            ui->actionRemove->setEnabled(true);
        }
        if(!pathInterpreter.isEmpty()) {
            ui->actionStartInterpreter->setEnabled(true);
        }
        if(!pathRecompiler.isEmpty()) {
            ui->actionStartRecompiler->setEnabled(true);
        }

        // load the config and fill in the overview pane
        ui->label_MachineLocation->setText(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).directory);

        Config local_config;
        Model local_model;

        rpcemu_set_datadir(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).directory.toUtf8().constData());
        config_load(&local_config, &local_model);

        ui->label_machine->setText(models[local_model].name_gui);
        ui->label_ram->setText(QString::number(local_config.mem_size) + " MB");
        ui->label_vram->setText(QString::number(local_config.vram_size) + " MB");
        ui->label_refresh->setText(QString::number(local_config.refresh) + " Hz");

        const char * network_names[] = {
            "None",
            "Network Address Translation",
            "Ethernet Bridging",
            "IP Tunnelling"
        };
        ui->label_networking->setText(network_names[local_config.network_type]);

        ui->overview_groupBox->setVisible(true);
    }
}

/**
 * User wants to create a new machine
 */
void
MainWindow::new_triggered()
{
    machinenew_dialog->reset();
    machinenew_dialog->exec(); // Modal
}

/**
 * User wants to import an existing machine
 */
void
MainWindow::import_triggered()
{
    machineimport_dialog->reset();
    machineimport_dialog->exec(); // Modal
}

/**
 * User wants to configure a machine
 */
void
MainWindow::settings_triggered()
{
    if((machine_list_current_row < 0)
       || (static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()))
    {
        error("MainWindow::settings_triggered: The currently selected row in the machines list is outside the range of the machine_instances array. This should never happen");
        return;
    }

    machinesettings_dialog->InitialiseMachine(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).directory.toUtf8().constData());

    machinesettings_dialog->exec(); // Modal

    // Use this to update the main window overview of the machine
    machine_list_item_selection_changed();
}

/**
 * User wants to remove or delete a machine
 */
void
MainWindow::remove_triggered()
{
    if((machine_list_current_row < 0)
       || (static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()))
    {
        error("MainWindow::remove_triggered: The currently selected row in the machines list is outside the range of the machine_instances array. This should never happen");
        return;
    }

    // Can't remove the 'local install' machine, it shouldn't be possible to click the 'Remove' button for
    // local install as it will be greyed out, but just in case.
    if(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).isLocalInstall) {
        return;
    }

    // Remove or Delete?
    QMessageBox msgBox(QMessageBox::Question,
        "RPCEmu",
        "Would you like to remove or delete this machine? Removed machines can be reimported, deleted machines lose all data",
        QMessageBox::Cancel,
        this);
    QPushButton *remove_button = msgBox.addButton("Remove", QMessageBox::ActionRole);
    QPushButton *delete_button = msgBox.addButton("Delete", QMessageBox::ActionRole);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.exec();

    if (msgBox.clickedButton() != remove_button
            && msgBox.clickedButton() != delete_button)
    {
        // They pressed cancel
        return;
    }

    if (msgBox.clickedButton() == remove_button) {
        // They pressed remove
        config_machine_remove(machine_instances.at(static_cast<unsigned>(machine_list_current_row)).name,
                              machine_instances.at(static_cast<unsigned>(machine_list_current_row)).directory);

        return;
    }

    if (msgBox.clickedButton() == delete_button) {
        // They pressed delete
        // Ask them if they're really really sure?
        QMessageBox msgBox2(QMessageBox::Question,
            "RPCEmu",
            "This will completely remove the directory " + machine_instances.at(machine_list_current_row).directory + " including all files and subdirectories, including your HostFS and harddisc images. Are you certain "
            "you which to delete this machine?",
            QMessageBox::Cancel,
            this);
        QPushButton *delete_button2 = msgBox2.addButton("Delete", QMessageBox::ActionRole);
        msgBox2.setDefaultButton(QMessageBox::Cancel);
        msgBox2.exec();

        if (msgBox2.clickedButton() == delete_button2) {
            // Delete Directory
            QDir dir(machine_instances.at(machine_list_current_row).directory);
            dir.removeRecursively();

            // Remove machine from lists of machines and config
            config_machine_remove(machine_instances.at(machine_list_current_row).name,
                                  machine_instances.at(machine_list_current_row).directory);
        }

        return;
    }
}

/**
 * User wants to run a machine
 */
void
MainWindow::startInterpreter_triggered()
{
    if ((machine_list_current_row < 0)
       || (static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()))
    {
        error("MainWindow::startInterpreter_triggered: The currently selected row in the machines list is outside the range of the machine_instances array. This should never happen");
        return;
    }

    if (pathInterpreter.isEmpty()) {
        // This shouldn't happen as the button should be greyed out
        return;
    }

    QStringList arguments;
    arguments << "--datadir" << machine_instances.at(machine_list_current_row).directory;

    QProcess *myProcess = new QProcess();
    myProcess->startDetached(pathInterpreter, arguments);
}

/**
 * User wants to run a machine
 */
void
MainWindow::startRecompiler_triggered()
{
    if ((machine_list_current_row < 0)
       || (static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()))
    {
        error("MainWindow::startRecompiler_triggered: The currently selected row in the machines list is outside the range of the machine_instances array. This should never happen");
        return;
    }

    if (pathRecompiler.isEmpty()) {
        // This shouldn't happen as the button should be greyed out
        return;
    }

    QStringList arguments;
    arguments << "--datadir" << machine_instances.at(machine_list_current_row).directory;

    QProcess *myProcess = new QProcess();
    myProcess->startDetached(pathRecompiler, arguments);
}

/**
 * Open the folder where the machines settings and files are stored
 */
void
MainWindow::openFolder_triggered()
{
    if ((machine_list_current_row < 0)
       || (static_cast<unsigned>(machine_list_current_row) >= machine_instances.size()))
    {
        error("MainWindow::openFolder_triggered: The currently selected row in the machines list is outside the range of the machine_instances array. This should never happen");
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile( machine_instances.at(machine_list_current_row).directory)) ;
}

/**
 * Add a named machine to the UIs machine list
 *
 * @param name machine name
 * @param front true if add to front, back otherwise
 */
void
MainWindow::add_machine_to_list(QString name, bool front)
{
    QIcon icon = QIcon(":/i/rpcemu_icon.png");
    QListWidgetItem *new_item = new QListWidgetItem(icon, name);

    if (front) {
        ui->machine_list->insertItem(0, new_item);
    } else {
        ui->machine_list->insertItem(ui->machine_list->count() +1 , new_item);
    }
}

/**
 * Remove the named machine from the UIs machine list
 *
 * @param name machine name
 */
void
MainWindow::remove_machine_from_list(QString name)
{
    int i;
    QListWidgetItem *item = nullptr;

    // Step through the UI machine list looking for a match on name
    for (i = 0; i < ui->machine_list->count(); i++) {
        item = ui->machine_list->item(i);

        if (0 == QString::compare(name, item->text())) {
            break;
        }

        item = nullptr;
    }

    // If found, pull it out
    if (item != nullptr) {
        ui->machine_list->clearSelection();
        item = ui->machine_list->takeItem(i);
        delete item;
    }
}
