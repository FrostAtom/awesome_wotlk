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

## NAME_PLATE_ADDED`Event`
Parameters: **unitId**`string`

Notifies that a new nameplate appeared

## NAME_PLATE_REMOVED`Event`
Parameters: **unitId**`string`

Notifies that a nameplate will be hidden

## nameplateDistance`CVar`
Arguments: **distance**`number`
Default: **43**

Sets the display distance of nameplates in yards