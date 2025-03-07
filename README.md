# Close/Restart Game Plugin
Restart or close a game from the plugin config menu, even while the HOME Button Menu is inaccessible (you may not notice this at first as the options aren't shown in the Wii U Menu).

Also adds button shortcuts to close or restart to the HOME Button Menu (press X to close, hold X to restart). These can be disabled in the settings.

## Installation
Download the latest release from the [Releases page](https://github.com/Lynx64/CloseRestartGamePlugin/releases) by clicking on `CloseRestartGamePlugin.wps`.<br/>
Copy the `CloseRestartGamePlugin.wps` file into `wiiu/environments/[ENVIRONMENT]/plugins`,<br/>
where [ENVIRONMENT] is the actual environment name (most likely 'aroma').

## Usage
Open the plugin config menu by pressing L, DPAD Down and Minus on the GamePad, Pro Controller or Classic Controller, or B, DPAD Down and Minus on a Wii Remote.

- 'Press X to close' is disabled by default to avoid accidental presses.

## Building
For building you need:
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/devkitPro/wut)
- [libnotifications](https://github.com/wiiu-env/libnotifications)

then run `make`
