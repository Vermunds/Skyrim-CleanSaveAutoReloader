# Clean Save Auto-Reloader
[![Nexus Mods](https://img.shields.io/badge/NexusMods-Download-orange)](https://www.nexusmods.com/skyrimspecialedition/mods/88219)
[![GitHub release](https://img.shields.io/github/v/release/Vermunds/Skyrim-CleanSaveAutoReloader)](https://github.com/Vermunds/Skyrim-CleanSaveAutoReloader/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](./LICENSE)

A mod for The Elder Scrolls V: Skyrim - Special Edition.

Loading a save on top of an already running game session is a well known source of script lag and save bloat. This mod makes every load a clean one: when you load a save, the game is restarted and the save is loaded automatically in the fresh process, skipping the intro and the main menu.

## How it works
The mod ships two components, both installed into `Data/SKSE/Plugins`:

- `SkyrimAutoReloader.dll` - the SKSE plugin. It intercepts save loading, works out the command line the game was originally launched with (including the SKSE loader and any alternate executable), and hands over to the helper.
- `SkyrimAutoReloaderHelper.exe` - a small helper process. It waits for the old game process to exit, then relaunches the game and passes the save to load through the `SKYRIM_AUTOLOAD_FILE_NAME` environment variable.

The helper is required because the game cannot relaunch itself while it is still shutting down. Both files must be present for the mod to work.

## Configuration
The settings can be changed in-game, in the settings menu of either SKSE Menu Framework or Fuzz's Legally Intelligible Core Kit, or in `Data/SKSE/Plugins/SkyrimAutoReloader.ini`. Changes made in the menu are saved to the same file.

## Download
Available on [Nexusmods](https://www.nexusmods.com/skyrimspecialedition/mods/88219).

## Build
To build this mod refer to my wrapper project [here](https://github.com/Vermunds/SkyrimSE-Mods).

## License
This software is available under the MIT License. See LICENSE for details.
