// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorHUD.h"
#include "ShipBuilding/ShipPart.h"
#include "ShipEditorPlayerController.h"

void AShipEditorHUD::BeginPlay()
{
	Super::BeginPlay();

	AShipEditorPlayerController* PC = Cast<AShipEditorPlayerController>(GetOwningPlayerController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get pc"));
		return;
	}

	UShipPartFactory* SPF = PC->GetShipPartFactory();
	check(SPF);
	if (!SPF->HasLoadedAssetData())
	{
		UE_LOG(LogTemp, Error, TEXT("SPF hasn't loaded asset data"));
		return;
	}

	PopulateShipParts(SPF->GetShipPartData());
}

void AShipEditorHUD::PopulateShipParts_Implementation(const TArray<FShipPartData>& ShipPartData)
{

}
