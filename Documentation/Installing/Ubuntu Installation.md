# Ubuntu - Installing KDL

## Preliminary information
This guide covers installing KDL on macOS. At present KDL has only been tested
on Ubuntu 16.04 and later. If you are using an earlier system, then we can
not guarantee everything to be fully working or compatible. Please be sure to report 
any issues with compatibility on other distros and earlier versions.

Before installing make sure you head over to the [releases page](https://github.com/tjhancocks/kdl/releases) and download the latest version for your operating system.

## Requirements
KDL has no dependancies that you need to worry about at this time. The installer
will handle everything that it needs.

## Installing
After downloading the latest release, extract the contents of the archive and open
the resulting `Linux` directory. The contents of the directory should be as follows...

![Ubuntu-LinuxDirectoy](https://user-images.githubusercontent.com/681356/79710235-47e6ec00-82bc-11ea-81f4-39118a3b28eb.png)

In order to perform the installation you'll need to open this directory in the terminal
and type in the following command:

```sh
sudo ./Install
```

In order to install the actual `kdl` binary on to your system the script needs superuser
privileges. The script will kick off the installation process and once it completes it
will look something like this: 

![Ubuntu-Terminal](https://user-images.githubusercontent.com/681356/79710237-49181900-82bc-11ea-8858-a400cf9de2ca.png)

The last step is to add the `/usr/local/bin` directory to your path. This can be
achieved by adding the following to your shell configuration

```sh
export PATH=/usr/local/bin:$PATH
```

## Installation Summary
The KDL installation installs the following files on to your system:

- **The `kdl` binary.**
  This is installed into `/usr/local/bin`. If an existing copy of KDL already
  exists, then it will be overwritten.

- **A default configuration.**
  The default configuration is installed at `~/.config.kdl`. This file contains
  information about where KDL can find type definitions for different scenarios,
  as well as some common type definitions found across multiple scenarios. You
  can add your own common type definitions here. This file will not be touched
  if it already exists.

- **Type definitions for EV Nova.**
  The installer will also put in place a set of type definitions for EV Nova.
  These are located at `~/.kdl/evn`, and will be rewritten by the installer if
  they already exist.

## Reporting Issues
If you find any issues with KDL, then please be sure to raise an issue so that
it can be investigated and fixed.

