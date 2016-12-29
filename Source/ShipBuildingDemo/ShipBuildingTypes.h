#pragma once

#include "ShipBuildingTypes.generated.h"

// TODO: find a more friendly way for defining part types.
UENUM(BlueprintType)
enum class EPartType : uint8
{
	PT_Fuselage, // Main body
	PT_Cockpit,
	PT_Engine,
	PT_Thruster,
	PT_Accessory, // Antenna etc.
	PT_MAX
};
