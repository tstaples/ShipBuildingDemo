// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "ShipBuilding/ShipPartFactory.h"
#include "ShipEditorHUD.generated.h"


/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API AShipEditorHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category=AShipEditorHUD)
	void PopulateShipParts(const TArray<FShipPartData>& ShipPartData);
};
