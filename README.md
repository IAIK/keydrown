# KeyDrown Proof-of-Concept


KeyDrown consists of three layers.

1. Layer: core layer, responsible for interrupt and key injection
2. Layer: protects the input handling library (libgtk/libgdk on x86, libinput on Android)
3. Layer: protects the UI widgets used in applications.

We support the following setups:

 * Ubuntu 16.04 x86_64
 * LG Nexus 5 with Android 6.0.1
 * OnePlus 3t with LineageOS 7.1.1

**For a quick start on Ubuntu 16.04, see section [Quick Start](#quick-start).**

## Table of content

* [Prerequisites](#prerequisites)
* [Building KeyDrown from Source](#building-keydrown-from-source)
* [Installing KeyDrown](#installing-keydrown)
* [Using KeyDrown](#using-keydrown)
* [Quick Start](#quick-start)
* [FAQ](#faq)

## Prerequisites

### x86
We provide all layers as binaries or Debian packages that can be readily used on a stock Ubuntu 16.04.

To compile the sources, the following packages are required:

* gcc
* make
* libgtk3
* Linux kernel headers

On Ubunut they can be installed using the packet manager:

	sudo apt-get install gcc make libgtk-3-dev linux-headers-$(uname -r)


### Android

Building the Android version (either for the Nexus 5 or the OnePlus 3t) requires a gcc cross-compiler for ARM.
The easiest way is to install the Android NDK which contains the complete toolchain.

 *  [Android NDK](https://developer.android.com/ndk/index.html) - Android Native Development Kit


## Building KeyDrown from Source

**If you want to use the pre-built binaries for Ubuntu, you can skip this section.**

### First Layer

The first layer is the core kernel module that is also required for all other layers. The source base is the same for x86 and ARM, thus there is only one folder.

#### x86

Simply run

    make

(or

    make x86

if you want to be explicit.)

#### Android

 To compile for the Nexus 5, run

    make hammerhead

To compile for the OnePlus 3t, run

    make oneplus3t

### Second Layer

The second layer is split into two versions, to be found in the *arm* and *x86* folder for Android and x86 respectively.

#### x86
The second layer is a GTK application that can simply be compiled by running

    make

 inside the *x86* folder.

#### Android

As the input library is tightly coupled with the Android base-system, the Android system has to be recompiled with the patched input library.
Simply apply the patch inside the *arm* folder to the Android source and recompile it.

### Third Layer

The third layer is again split into two versions, to be found in the *arm* and *x86* folder for Android and x86 respectively.

#### x86

The third layer is a shared library that can simply be compiled by running

    make

#### Android

The third layer is a patch to the AnySoft Keyboard.
To compile it, clone the AnySoft Keyboard source code, apply the patch and compile it using Android Studio.

    git clone https://github.com/AnySoftKeyboard/AnySoftKeyboard.git
    cd AnySoftKeyboard
    git apply ../anysoftkeyboard.patch


## Installing KeyDrown

### First Layer

#### x86

The Debian package can be installed using the UI by opening the *.deb* files or using the console by running `sudo dpkg -i keydrown-0.1.deb keydrown1.deb`. 

If the first layer was compiled from source, run

    sudo make install


#### Android

Copy the compiled kernel module to */data/local/tmp* on the phone:

    adb push keydrown.ko /data/local/tmp


### Second Layer

#### x86

The Debian package can be installed using the UI by opening the *.deb* file or using the console by running `sudo dpkg -i keydrown2.deb`.

If the first layer was compiled from source, run

    sudo make install


No further action is required if the second layer is allowed to run as root.
Otherwise, create a new user and add the user to the *input* group.

    sudo useradd -G input keydrown



#### Android

No action required if the modified input library was compiled and flashed.

### Third Layer

#### x86

The Debian package can be installed using the UI by opening the *.deb* file or using the console by running `sudo dpkg -i libgtk-keydrown.deb`.

If the third layer was compiled from source, run

    sudo make install

#### Android

The *.apk* for the second layer can either be installed using a package installation app on the phone directly, or via adb:

    adb install anysoftkeyboard_keydrown.apk


## Using KeyDrown

### First Layer

#### x86

To start KeyDrown, run

    sudo service keydrown start

Run

    watch -n 0.1 grep " 1:.*i8042" /proc/interrupts

to see a uniform increase in all values instead of an increase only when a key is pressed.
To stop KeyDrown, run

    sudo service keydrown stop


#### Android

Ensure the KeyDrown kernel module is loaded by running

    su
    cd /data/local/tmp
    insmod keydrown.ko

To start KeyDrown, run

    echo 1 > /proc/keydrown

On the Nexus 5, run

    watch -n 0.1 grep " 362:" /proc/interrupts

On the OnePlus 3T, run

    watch -n 0.1 grep " 413:" /proc/interrupts



to see a uniform increase in all values instead of an increase only when the screen is touched.
To stop KeyDrown, run

    echo 0 > /proc/keydrown

### Second Layer

#### x86

Start the second layer by running

    sudo keydrown2

or

    sudo su - keydrown -c keydrown2

if KeyDrown should not be running as root user.

The second layer creates a window on startup which asks to press a key on the keyboard which should be protected. As soon as the windows receives a key press, it closes itself and starts protecting libgtk.

#### Android

The second layer is always active if the first layer is active.

### Third Layer

#### x86

The third layer can be enabled per application.
To protect text input for an application, simply start the application by running

    keydrown3 <application>

If KeyDrown protects the application, it will print `[info] KeyDrown active` to the console on startup.

#### Android

As long as the KeyDrown-enabled AnySoftKeyboard is used, every text input is protected.

## Quick Start

The easiest way to test KeyDrown is to install the pre-built binaries from the *ubuntu-deb* folder on Ubuntu 16.04.

### Install

    sudo dpkg -i *.deb

### Run

    sudo modprobe keydrown
    sudo keydrown2 &
    sudo service keydrown start

To start an application with the third layer protection, run

    keydrown3 <application>

### Stop

    sudo service keydrown stop

### Uninstall

    sudo rmmod keydrown
    sudo dkms remove -m keydrown -v 0.1 --all
    sudo dpkg -r keydrown1
    sudo dpkg -r keydrown2
    sudo dpkg -r libgtk-keydrown


## FAQ

* **I cannot install the pre-built first layer.**

    If your kernel version is not 4.6, the package has to recompile itself. Ensure that you have the Linux kernel headers of your kernel installed (`sudo apt-get install linux-headers-$(uname -r)`).

* **When installing the kernel module, I get some strange error about secure boot.**

    This error can be ignored. However, if you update your kernel, you might have to run `sudo dpkg-reconfigure keydrown-dkms`.

* **I do not see any interrupts in */proc/interrupts* when typing.**

  Ensure that you are typing with your laptop keyboard, not an external USB keyboard. USB keyboards are not supported in this PoC.

* **Interrupts are not injected for all cores, only for one/some.**

  Ensure the IRQ affinity is set correctly in */proc/irq/1/smp_affinity*. When in doubt, run `sudo su; echo 01 > /proc/irq/1/smp_affinity; exit` and reload the KeyDrown module (`sudo rmmod keydrown && sudo modprobe keydrown`). This disables load balancing, and all interrupts (real and fake) are only handled by the first core.

* **The second layer / third layer does not do anything!**

    Both the second layer and the third layer require that the kernel module (first layer) is loaded and active.

* **How do I uninstall KeyDrown?**

    If KeyDrown was built from source, simply run

        sudo make uninstall

    for every layer. If KeyDrown was installed using the binary packages, then see the Uninstall subsection of section Quick Start.
