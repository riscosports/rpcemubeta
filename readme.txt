
Build instructions for macOS using MacPorts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Go to https://www.macports.org/install.php and download the version that matches your OS versiom.
Run the installer and follow the wizard to install.
Once installed open a terminal and type,

export PATH=/opt/local/bin:$PATH

type gcc -v. If the xcode command line tools aren’t installed you should get prompted to install them.

sudo port selfupdate

sudo port install qt5

git clone https://github.com/riscosports/rpcemubeta.git

Then build rpcemu as normal. (the qmake command is at /opt/local/libexec/qt5/bin/qmake)

RPCEmu
~~~~~~

RPCEmu is an emulator of Acorn's Risc PC and A7000 machines. It is a work in
progress and should be considered of Alpha Quality.

The latest version is available from, this also has links to compilation
instructions for various platforms.

    http://www.marutan.net/rpcemu/

The User Manual is available from

    http://www.marutan.net/rpcemu/manual/

RPCEmu requires a RISC OS ROM image to work; check here for details:

    http://www.marutan.net/rpcemu/manual/romimage.html

RPCEmu is licensed under the GPL, see COPYING for more details.

