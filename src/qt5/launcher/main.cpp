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
#include <assert.h>

#include <iostream>
#include <vector>

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QSettings>

#include "main.h"
#include "mainwindow.h"
#include "rpcemu.h"
#include "cmos.h"

std::vector<MachineInstance> machine_instances = {};

static void config_machine_list_load();
static bool copyDirectoryNested(QString from, QString to);

static MainWindow *w;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc != 1) {
        fprintf(stderr, "rpcemu launcher does not take any arguments\n");
        exit(EXIT_FAILURE);
    }

    // Add a program icon
    QApplication::setWindowIcon(QIcon(":/i/rpcemu_icon.png"));

    // Load the list of machines from the config file
    config_machine_list_load();

    w = new MainWindow();
    w->show();

    // Determine if we have a 'local install' alongside the launcher binary path
    QString localDir = a.applicationDirPath();
    QString localRpcCfg =  localDir + "/rpc.cfg";
    if (QFileInfo::exists(localRpcCfg)) {
        config_machine_import("Local Install", localDir, true);
    }

    // Search the path alongside rpcemu.exe looking for the recompiler and interpreter version
    // This should track down the binaries if they're in the 'portable' install or in a theoretical
    // 'systemwide' install
    // The findExecutable() method is smart enough to search case insensitive on windows and check for
    // .exe file extension
    QStringList pathsToSearch = QStringList(localDir);
    w->pathInterpreter = QStandardPaths::findExecutable("rpcemu-interpreter", pathsToSearch);
    w->pathRecompiler  = QStandardPaths::findExecutable("rpcemu-recompiler",  pathsToSearch);

    if (w->pathInterpreter.isEmpty() && w->pathRecompiler.isEmpty()) {
        QMessageBox msgBox;

        msgBox.setText("Unable to locate the rpcemu-interpreter or rpcemu-recompiler programs. Unable to continue.");
        msgBox.exec();

        exit(EXIT_FAILURE);
    }

    return a.exec();
}

/**
 * Parse and load NAT port forwarding rules into the global list
 */
static void
config_machine_list_load()
{
    QSettings settings(QDir::home().filePath("RPCEmu/machines.ini"), QSettings::IniFormat);
    const int size = settings.beginReadArray("machines");

    for (int i = 0; i < size; i++) {
        MachineInstance machine;

        settings.setArrayIndex(i);

        machine.name      = settings.value("name", "").toString();
        machine.directory = settings.value("directory", "").toString();
        machine.isLocalInstall = false; // Local install is not stored in the machines.ini file

        // Validation
        // Reject entries that don't contain enough data
        if(machine.name.isEmpty() || machine.directory.isEmpty()) {
            error("config_machine_list_load: Entry missing name or directory string, removing");
            continue;
        }

        // Reject entries where the directory listed isn't there anymore
        QFileInfo checkDir(machine.directory);
        if(false == checkDir.exists() || false == checkDir.isDir()) {
            error("config_machine_list_load: Entry '%s' directory '%s' no longer exists, removing", machine.name.toStdString().c_str(), machine.directory.toStdString().c_str());
            continue;
        }

        // Reject entries where the directory doesn't contain an rpc.cfg file anymore
//        QString path = machine.directory + "/rpc.cfg";
//        error("%s", path.toStdString().c_str());
        QFileInfo checkConfig(machine.directory + "/rpc.cfg");
        if(false == checkConfig.exists() || false == checkConfig.isFile()) {
            error("config_machine_list_load: Entry '%s' doesn't have an rpc.cfg file in '%s', removing", machine.name.toStdString().c_str(), machine.directory.toStdString().c_str());
            continue;
        }

        // Passed validation store, in list of machines
        machine_instances.push_back(machine);
    }
    settings.endArray();
}

/**
 * Store NAT port forwarding rules from the global list
 */
static void
config_machine_list_save()
{
    QSettings settings(QDir::home().filePath("RPCEmu/machines.ini"), QSettings::IniFormat);
    int itemnum = 0;

    settings.beginWriteArray("machines");
    for (MachineInstance machine : machine_instances) {
        if (false == machine.isLocalInstall) {
            settings.setArrayIndex(itemnum);
            settings.setValue("name",      machine.name);
            settings.setValue("directory", machine.directory);
            itemnum++;
        }
    }
    settings.endArray();
}

/**
 * Attempt to create a new specified machine, and fill in lots of
 * defaults for it.
 *
 * @param name New machines name
 * @param directory Absolute directory including the machine name
 * @return bool of success
 */
bool
config_machine_new(QString name, QString directory)
{
    QDir path = QDir(directory);

    // Create directory
    if (false == path.mkpath(directory)) {
        QMessageBox msgBox;
        msgBox.setText("Unable to create directory for the new machine");
        msgBox.exec();
        return false;
    }

    // Copy in resources
    if (false == copyDirectoryNested(":/default", directory)) {
        QMessageBox msgBox;
        msgBox.setText("Unable to copy all default resources for the new machine");
        msgBox.exec();

        // Tidy up. Remove the directory we created in the step above, and any files that
        // did copy
        path.removeRecursively();

        return false;
    }

    // Add machine to machine list
    MachineInstance machine;

    machine.name           = name;
    machine.directory      = directory;
    machine.isLocalInstall = false;
    machine_instances.push_back(machine);

    // Update UI list of machines
    w->add_machine_to_list(name, false);

    // Save list of machines
    config_machine_list_save();

    return true;
}


/**
 * Attempt to create a new specified machine, and fill in lots of
 * defaults for it.
 *
 * @param name New machines name
 * @param directory Absolute directory of machine
 * @param isLocalInstall bool of whether the machine is the one alongside the rpcemu binaries
 * @return bool of success
 */
bool
config_machine_import(QString name, QString directory, bool isLocalInstall)
{
    // Add machine to machine list
    MachineInstance machine;

    machine.name           = name;
    machine.directory      = directory;
    machine.isLocalInstall = isLocalInstall;

    if(isLocalInstall) {
       machine_instances.insert(machine_instances.begin(), machine);
    } else {
       machine_instances.push_back(machine);
    }

    // Update UI list of machines
    w->add_machine_to_list(name, isLocalInstall);

    // Save list of machines
    config_machine_list_save();

    return true;
}


/**
 * Remove a machine from the config and from the UI
 *
 * @param name Machine name for matching
 * @param directory Machine directory for matching
 */
bool
config_machine_remove(QString name, QString directory)
{
    // Step through conf
    int i = 0;

    for (MachineInstance machine : machine_instances) {
        if (0 == QString::compare(machine.name, name)
           && 0 == QString::compare(machine.directory, directory))
        {
            if (false == machine.isLocalInstall) {
                machine_instances.erase(machine_instances.begin() + i);

                // Update UI list of machines
                w->remove_machine_from_list(name);

                // Save list of machines
                config_machine_list_save();

            } else {
                // Error Attempt to delete the local install that's next to the binaries, this would be very bad indeed
            }
            break;
        }
        i++;
    }

    return true;
}

/**
 * Copy a directory tree (from) to new location (to)
 *
 * Based upon code from saurabhjadhav1911, believed to be in the public domain
 * https://forum.qt.io/topic/75440/copy-file-from-resource-to-disk-keeping-folder-structure-platform-independant/7
 *
 * @param from Copy files from here
 * @param to Copy files to here
 * @returns bool of whether there were any failures copying files
 */
bool
copyDirectoryNested(QString from,QString to)
{
    QDirIterator it(from, QDirIterator::Subdirectories);

    while (it.hasNext()) {

        QString file_in = it.next();

        QFileInfo file_info = QFileInfo(file_in);

        QString file_out = file_in;
        file_out.replace(from,to);

        if (file_info.isFile()) {
            // is file copy
            if (false == QFile::copy(file_in, file_out)) {
                return false;
            }

            // Work around files being created read only on windows
            QFile(file_out).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }

        if (file_info.isDir()) {
            //dir mkdir
            QDir dir(file_out);
            if (!dir.exists()) {
                if (false == dir.mkpath(".")) {
                     return false;
                }
            }
        }
    }

    return true;
}

// Replacement versions of RPCEmu binary functions needed for using settings.cpp
static FILE *arclog; /* Log file handle */
static char datadir[512] = "./";
static char logpath[1024] = "";

/**
 * Return the path of the data directory containing all the sub data parts
 * used by the program, eg romload, hostfs etc.
 *
 * @return Pointer to static zero-terminated string of path
 */
const char *
rpcemu_get_datadir(void)
{
    return datadir;
}

/**
 * Set the datadirectory to the specified path
 * used to handle multiple machines in different locations
 */
void
rpcemu_set_datadir(const char *newdir)
{
    // Add on a trailing slash incase the path is mising it
    if (snprintf(datadir, sizeof(datadir), "%s/", newdir) >= (int) sizeof(datadir)) {
        rpclog("rpcemu_set_datadir(): Path too long");
        exit(EXIT_FAILURE);
    }
}

/**
 * Return the full path to the RPCEmu log file.
 *
 * @return Pointer to static zero-terminated string of full path to log file
 */
const char *
rpcemu_get_log_path(void)
{
    if (logpath[0] == '\0') {
        QString path = QDir::home().filePath("RPCEmu/rpclauncherlog.txt");

        strcpy(logpath, path.toStdString().c_str());
    }

    return logpath;
}

/**
 * Report a non-fatal error to the user.
 * The user then clicks the continue button to carry on using the program.
 *
 * @param format varargs format
 * @param ... varargs arguments
 */
void
error(const char *format, ...)
{
    char buf[4096];
    va_list ap;

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    rpclog("ERROR: %s\n", buf);
    fprintf(stderr, "RPCEmu error: %s\n", buf);
}

/**
 * Write a message to the RPCEmu log file rpclog.txt
 *
 * @param format printf style format of message
 * @param ...    format specific arguments
 */
void
rpclog(const char *format, ...)
{
    va_list arg_list;

    assert(format);

    if (arclog == NULL) {
        arclog = fopen(rpcemu_get_log_path(), "wt");
        if (arclog == NULL) {
            return;
        }
    }

    va_start(arg_list, format);
    vfprintf(arclog, format, arg_list);
    va_end(arg_list);

    fflush(arclog);
}
