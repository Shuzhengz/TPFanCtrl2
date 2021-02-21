# TPFanCtrl2

## About

This is a fork of the [original TPFC](https://github.com/ThinkPad-Forum/TPFanControl/tree/master/fancontrol), and is based on [byrnes' dual fan mod](https://github.com/byrnes/TPFanControl). It's updated it to work with two fan devices, such as the X1 Extreme. This has only been tested on my X1 Extreme Gen. 1, but it should work on any other dual-fan Thinkpads that are released. The default fan profile that is included is a silent one, with the fans only coming on at 60c. This can be changed by editing tpfancontrol.ini in the fancontrol/Debug. Visual Studio 2019 Community was used to build and debug.

## Requirements

To avoid errors, either install [tvicport](https://www.entechtaiwan.com/dev/port/index.shtm) manually or install the original version of TPFanControl found [here](https://sourceforge.net/projects/tp4xfancontrol/), and run the dual-fan version instead of the original version.

## Running at startup

The easiest way to run TPFC at startup is:

- Right-click on fancontrol.exe and select copy
- Press Windows-r or search for run in the start menu
- Type `shell:startup` in the run box
- Right click in the window that opens and select paste shortcut

Note: This won't start TPFC until you reboot.
