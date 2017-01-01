#pragma once

#include "ShipBuildingTypes.generated.h"

// TODO: find a more friendly way for defining part types.
UENUM(BlueprintType)
enum class EPartType : uint8
{
	PT_Fuselage		UMETA(DisplayName = "Fuselage"), // Main body
	PT_Cockpit		UMETA(DisplayName = "Cockpit"),
	PT_Engine		UMETA(DisplayName = "Engine"),
	PT_Thruster		UMETA(DisplayName = "Thruster"),
	PT_Accessory	UMETA(DisplayName = "Accessory"), // Antenna etc.
	PT_MAX
};
