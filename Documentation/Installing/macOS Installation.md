# macOS - Installing KDL

## Preliminary information
This guide covers installing KDL on macOS. At present KDL has only been tested
on macOS 10.14 Mojave and later. If you are using an earlier system, then we can
not guaruntee everything to be fully working or compatible. We do however plan to
support back to macOS 10.12.

Before installing make sure you head over to the [releases page](https://github.com/tjhancocks/kdl/releases) and download the latest version for your operating system.

## Requirements
KDL has no dependancies that you need to worry about at this time. The installer
will handle everything that it needs.

## Installing
After downloading the latest release, extract the contents of the archive and open
the resulting macOS directory. The contents of the directory should be as follows...

<div style="text-align:center"><img src="https://user-images.githubusercontent.com/681356/79683481-5680c500-8222-11ea-9df4-c838ef623c98.png" /></div>

When you double click on the _Install_ icon the Terminal will launch and perform
the installation process. Once it completes, it will look something like this:

<div style="text-align:center"><img src="https://user-images.githubusercontent.com/681356/79683577-06eec900-8223-11ea-9c90-fc683e794e4e.png" /></div>

The last step is to add the `/usr/local/bin` directory to your path. This can be
acheived by adding the following to your shell configuration

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

