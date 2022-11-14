[C_NamePlates](#c_nameplate) - [Unit](#unit) - [Inventory](#inventory) - [Misc](#misc)

# C_NamePlate
Backported C-Lua interfaces from retail

## C_NamePlate.GetNamePlateForUnit`API`
Arguments: **unitId** `string`

Returns: **namePlate**`frame`

Get nameplates by unitId
```lua
frame = C_NamePlate.GetNamePlateForUnit("target")
```

## C_NamePlate.GetNamePlates`API`
Arguments: `none`

Returns: **namePlateList**`table`

Get all visible nameplates
```lua
for _, nameplate in pairs(C_NamePlate.GetNamePlates()) do
  -- something
end
```

## NAME_PLATE_CREATED`Event`
Parameters: **namePlateBase**`frame`

Fires when nameplate was created

## NAME_PLATE_UNIT_ADDED`Event`
Parameters: **unitId**`string`

Notifies that a new nameplate appeared

## NAME_PLATE_UNIT_REMOVED`Event`
Parameters: **unitId**`string`

Notifies that a nameplate will be hidden

## nameplateDistance`CVar`
Arguments: **distance**`number`

Default: **41**

Sets the display distance of nameplates in yards

# Unit

## UnitIsControlled`API`
Arguments: **unitId**`string`

Returns: **isControlled**`bool`

Returns true if unit being hard control

## UnitIsDisarmed`API`
Arguments: **unitId**`string`

Returns: **isDisarmed**`bool`

Returns true if unit is disarmed


## UnitIsSilenced`API`
Arguments: **unitId**`string`

Returns: **isSilenced**`bool`

Returns true if unit is silenced

# Inventory

## GetInventoryItemTransmog`API`
Arguments: **unitId**`string`, **slot**`number`

Returns: **itemId**`number`, **enchantId**`number`

Returns info about item transmogrification

# Misc

## FlashWindow`API`
Arguments: `none`

Returns: `none`

Starts flashing of game window icon in taskbar

## IsWindowFocused`API`
Arguments: `none`

Returns: `bool`

Returns 1 if game window is focused, overtwice nil

## FocusWindow`API`
Arguments: `none`

Returns: `none`

Raise game window

## CopyToClipboard`API`
Arguments: **text**`string`

Returns: `none`

Copies text to clipboard

## cameraFov`CVar`
Parameters: **value**`number`

Default: **100**

Сhanges the camera view area (fisheye effect), in range **1**-**200**
