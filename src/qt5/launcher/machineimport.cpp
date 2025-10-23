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
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "main.h"
#include "machineimport.h"
#include "ui_machineimport.h"

MachineImport::MachineImport(QWidget *parent) :
       QDialog(parent),
       ui(new Ui::MachineImport)
{
    ui->setupUi(this);

    setWindowTitle("Import an existing machine");

    // Connect actions to widgets
    connect(ui->pushButton, &QPushButton::clicked, this, &MachineImport::browse_folder_clicked);
}


/**
 * Reset the machine import dialog to defaults before it is shown
 */
void
MachineImport::reset()
{
    ui->lineEdit_name->setText("new machine");
    ui->lineEdit_location->setText(QDir::home().filePath("RPCEmu/Machines/"));
}

/**
 * User has closed the machine import diaglog box, process the result
 *
 * @param result status of which outcome (ok, cancelled etc) chosen
 */
void
MachineImport::done(int result)
{
    if (result == QDialog::Accepted) {

        QMessageBox msgBox;
        QString name = ui->lineEdit_name->text();
        QString path = ui->lineEdit_location->text();

        // check if values are present
        if (0 == name.compare("")) {
            msgBox.setText("You must specify a machine name");
            msgBox.exec();
            return;
        }
        if (0 == path.compare("")) {
            msgBox.setText("You must specify a machine location");
            msgBox.exec();
            return;
        }

        QDir directory = QDir(path);

        // check if name or dirctory is in existing list of machines
        for (MachineInstance machine : machine_instances) {
            if (0 == name.compare(machine.name)) {
                msgBox.setText("Machine name already exists, please choose another");
                msgBox.exec();
                return;
            }

            if (0 == directory.absolutePath().compare(machine.directory)) {
                msgBox.setText("Machine directory '" + machine.directory + "' already matches machine '" + machine.name + "', not importing again");
                msgBox.exec();
                return;
            }
        }

        QString confRpcCfg = path + "/rpc.cfg";

        // Check path has a rpc.cfg
        if (!QFileInfo::exists(confRpcCfg)) {
            msgBox.setText("Directory '" + path + "' does not contain an RPCEmu rpc.cfg files");
            msgBox.exec();
            return;
        }

        // Inform backend to make machines
        if (false == config_machine_import(name, directory.absolutePath(), false /* Is not a local install */)) {
            // Error messages will have been printed out already
            return;
        }
        rpclog("Import machine '%s' '%s'\n", name.toStdString().c_str(),
                path.toStdString().c_str());
    }

    QDialog::done(result);
}

/**
 * User clicked on the browse button next to the machine folder location display
 */
void
MachineImport::browse_folder_clicked()
{
    QString newdir = QFileDialog::getExistingDirectory(this,
                                                       tr("Choose location of existing machine"),
                                                       ui->lineEdit_location->text());

    if (false == newdir.isEmpty() && false == newdir.isNull()) {
        // User didn't hit 'cancel'
        ui->lineEdit_location->setText(newdir);
    }
}
