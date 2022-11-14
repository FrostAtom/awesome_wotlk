# Awesome WotLK
## World of Warcraft 3.3.5a 12340 improvements library

## <b> [Details](#details) - [Installation](#installation) - [Docs](https://github.com/FrostAtom/awesome_wotlk/blob/main/docs/api_reference.md) - [For suggestions](#for-suggestions) - [3rd party libraries](#3rd-party-libraries)

___
## Details
> - BugFix: clipboard issue when non-english text becomes "???"
> - Auto Login (Through cmdline/shortcuts, Usage: `Wow.exe -login "LOGIN" -password "PASSWORD" -realmlist "REALMLIST" -realmname "REALMNAME" `)
> - Changing cameras FOV
> - New API:
    - C_NamePlate.GetNamePlates
    - C_NamePlate.GetNamePlateForUnit
    - UnitIsControlled
    - UnitIsDisarmed
    - UnitIsSilenced
    - GetInventoryItemTransmog
    - FlashWindow
    - IsWindowFocused
    - FocusWindow
    - CopyToClipboard
> - New events:
    - NAME_PLATE_CREATED
    - NAME_PLATE_UNIT_ADDED
    - NAME_PLATE_UNIT_REMOVED
> - New CVars:
    - nameplateDistance
    - cameraFov<br>
See [Docs](https://github.com/FrostAtom/awesome_wotlk/blob/main/docs/api_reference.md) for details

## Installation
1) Download latest [release](https://github.com/FrostAtom/awesome_wotlk/releases)
2) Unpack files to root game folder
3) Launch `AwesomeWotlkPatch.exe`, you should get a message
4) To update just download and replace dll

## For suggestions
Join us at [Discord](https://discord.gg/NNnBTK5c8e) - [Telegram](https://t.me/wow_soft)
<br><img src="https://raw.githubusercontent.com/FrostAtom/awesome_wotlk/main/docs/assets/wow_soft.jpg" width="148" height="148">

## 3rd party libraries
- [microsoft-Detours](https://github.com/microsoft/Detours) - [license](https://github.com/microsoft/Detours/blob/6782fe6e6ab11ae34ae66182aa5a73b5fdbcd839/LICENSE.md)