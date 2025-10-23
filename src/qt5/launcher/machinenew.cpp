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
#include "machinenew.h"
#include "ui_machinenew.h"

MachineNew::MachineNew(QWidget *parent) :
       QDialog(parent),
       ui(new Ui::MachineNew)
{
    ui->setupUi(this);

    setWindowTitle("Create a new machine");

    // Connect actions to widgets
    connect(ui->pushButton, &QPushButton::clicked, this, &MachineNew::browse_folder_clicked);
}


/**
 * Reset the machine new dialog to defaults before it is shown
 */
void
MachineNew::reset()
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
MachineNew::done(int result)
{
    if (result == QDialog::Accepted) {
        QMessageBox msgBox;
        QString name = ui->lineEdit_name->text();
        QString path = ui->lineEdit_location->text();

        // check if values are present
        if (name.isEmpty()) {
            msgBox.setText("You must specify a machine name");
            msgBox.exec();
            return;
        }
        if (path.isEmpty()) {
            msgBox.setText("You must specify a machine location");
            msgBox.exec();
            return;
        }

        // check if name is in existing list of machines
        for (MachineInstance machine : machine_instances) {
            if (name.compare(machine.name) == 0) {
                msgBox.setText("Machine name already exists, please choose another");
                msgBox.exec();
                return;
            }
        }

        // check path doesn't already exist
        QDir directory = QDir(path + "/" + name);
        if (QFileInfo::exists(directory.absolutePath())) {
            QString message = QString("Machine path '%1' already exists, please choose another name or folder").arg(directory.absolutePath());
            msgBox.setText(message);
            msgBox.exec();
            return;
        }

        // OK actually try to make the machine!
        if (false == config_machine_new(name, directory.absolutePath())) {
            // Error messages will have been printed out already
            return;
        }
        rpclog("New machine '%s' '%s' '%s'", name.toStdString().c_str(),
                path.toStdString().c_str(),
                directory.absolutePath().toStdString().c_str());
    }

    QDialog::done(result);
}

/**
 * User clicked on the browse button next to the machine folder location display
 */
void
MachineNew::browse_folder_clicked()
{
    QString newdir = QFileDialog::getExistingDirectory(this,
                                                       tr("Choose location for new machine"),
                                                       ui->lineEdit_location->text());

    if (false == newdir.isEmpty() && false == newdir.isNull()) {
        // User didn't hit 'cancel'
        ui->lineEdit_location->setText(newdir);
    }
}
