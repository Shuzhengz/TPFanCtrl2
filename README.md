# TPFanCtrl2

## About

This is a fork of the [original TPFC](https://github.com/ThinkPad-Forum/TPFanControl/tree/master/fancontrol), and is based on [byrnes' dual fan mod](https://github.com/byrnes/TPFanControl). It's updated it to work with two fan devices, such as the X1 Extreme. This has only been tested on X1 Extreme Gen. 1, but it should work on any other dual-fan Thinkpads that are released. The default fan profile that is included is a silent one, with the fans only coming on at 60c. This can be changed by editing tpfancontrol.ini in the fancontrol/Debug. Visual Studio 2019 Community was used to build and debug.

## Requirements

To avoid errors, either install [tvicport](https://www.entechtaiwan.com/dev/port/index.shtm) manually or install the original version of TPFanControl found [here](https://sourceforge.net/projects/tp4xfancontrol/) (source code [here](https://github.com/ThinkPad-Forum/TPFanControl)), and run the dual-fan version instead of the original version.

Windows 10 is required to run the program. Windows 11 is currently untested, but should theoretically be compatible.

## Configurating

You can configure the behaviour of the program by editing the values inside the `TPFanControl.ini` file that comes with the program

The `TPFanControl.ini` file should always be in the same directory as the `fancontrol.exe` executable file

Example of configurable parameters:

- Temperature refresh cycle time
- Start minimized in the taskbar tray
- Max read errors before the program auto shuts down
- Log program log to file & log data to CSV file
- Configure the fan curve in smart mode
- Configure a second smart mode that is accessable in the tray
- Temperature threshold to exit the manual mode

## Running at startup (Not Recommended)

You can choose to run TPFC at startup:

- Right-click on fancontrol.exe and select copy
- Press Windows-r or search for run in the start menu
- Type `shell:startup` in the run box
- Right click in the window that opens and select paste shortcut

Note: This won't start TPFC until you reboot.


### License
The author claims no copyright, copyleft, license or whatsoever for the program itself. You may use, reuse or distribute its binaries or source code in any desired way or form, Useage of binaries or source shall be entirely and without exception at your own risk.
