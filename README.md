# TPFanCtrl2 <img src="https://raw.githubusercontent.com/Shuzhengz/TPFanCtrl2/main/fancontrol/res/app.ico" alt = "App Icon" width = "30"/>

ThinkPad Fan Control 2, a fan control utility software for ThinkPad users running Windows 10/11

## About

This is a fork of the [original TPFC](https://github.com/ThinkPad-Forum/TPFanControl/tree/master/fancontrol), and is based on [byrnes' dual fan mod](https://github.com/byrnes/TPFanControl). It's updated it to work with two fan devices, such as the X1 Extreme. This has only been tested on X1 Extreme Gen. 1, but it should work on any other dual-fan Thinkpads that are released. The default fan profile that is included is a silent one, with the fans only coming on at 60c. This can be changed by editing tpfancontrol.ini in the fancontrol/Debug.

Visual Studio 2022 Community was used to build and debug. A release build is in the works

<p align="center">
  <img src="https://raw.githubusercontent.com/Shuzhengz/TPFanCtrl2/master/fancontrol/res/TPFC2.png" alt="Example Photo" width="750"/>
</p>


## Requirements

To avoid errors, either install [tvicport](https://www.entechtaiwan.com/dev/port/index.shtm) manually or install the original version of TPFanControl found [here](https://sourceforge.net/projects/tp4xfancontrol/) (source code [here](https://github.com/ThinkPad-Forum/TPFanControl)), and run the dual-fan version instead of the original version.

Administration permission is required for the program to be able to control system fan speed.

Windows 10 is required to run the program. Windows 11 is currently untested and unsupported, but should theoretically be compatible.


## Configuring

You can configure the behaviour of the program by editing the values inside the `TPFanControl.ini` file that comes with the program,

The `TPFanControl.ini` file should always be in the same directory as the `fancontrol.exe` executable file.

Example of configurable parameters:

- Temperature refresh cycle time
- Start minimized in the taskbar tray
- Max read errors before the program auto shuts down
- Log program log to file & log data to CSV file
- Configure the fan curve in smart mode
- Configure a second smart mode that is accessable in the tray
- Temperature threshold to exit the manual mode

### Manual Fan Levels

| Normal: | Advanced:|
| --- | ----------- |
| 0 - Off | 0 - Off |
| 1 - 30% | 50 - 40% |
| 2 - 40% | 60 - 60% |
| 3 - 45% | 70 - 100% |
| 4 - 50% | 80 - 100% |
| 5 - 60% |
| 6 - 65% |
| 7 - 65% |

<h5>Advanced level value is converted into hex (i.e. 70 is 0x46), the value above is a rough estimate of what to expect</h5>

### Additional Information

You can find additional information here on the ThinkPad Wiki: [https://thinkwiki.de/TPFanControl](https://thinkwiki.de/TPFanControl).


## Building Instructions

Building this program requires Microsoft Visual Studio 2022 Community

The project should have the correct configuration upon opening the project file `fancontrol.sln`, though you might have to manually copy and paste the `.ini` file into the Debug folder to test the built program

The project currently builds with the configuration: `Debug` `Win32` (You will need admin permission to run the built program)

Do not build the project without TPFCIcon and TPFCIcon_noballons, they are necessary parts of the application and the main fan control software will not function without them


## Running at startup (Not Recommended)

You can choose to run TPFC at startup:

- Right-click on fancontrol.exe and select copy
- Press Windows-r or search for run in the start menu
- Type `shell:startup` in the run box
- Right click in the window that opens and select paste shortcut

Note: This won't start TPFC until you reboot.


## Uninstall

Delete the folder containing the program and the ini file.

That's it, there's no residual files.

If you added the program to run at startup, you will have to also delete the shortcut from the start up folder.

## Known Issues

Sometimes the fans will fail to sync, with one ramping up and the other not.
This will usually fix itself, but to fix it manually, switch to BIOS mode, then switch back to the desired mode

It will also sometimes take a longer time for the speed to update and the fans to spin up.
This is caused by EC management for the newer ThinkPad BIOS. There is currently no way around it.

## License
This program has an Unlicense license

The author claims no copyright, copyleft, license or whatsoever for the program itself. You may use, reuse or distribute its binaries or source code in any desired way or form, Useage of binaries or source shall be entirely and without exception at your own risk.
