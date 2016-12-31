// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipPartFactory.h"
#include "ShipPart.h"

UShipPartFactory::UShipPartFactory()
	: bAssetDataLoaded(false)
{

}

void UShipPartFactory::Init(const FString& RootShipPartPath)
{
	if (bAssetDataLoaded)
		return;

	ShipPartLibrary = UObjectLibrary::CreateLibrary(AShipPart::StaticClass(), true, false);
	const int32 NumLoaded = ShipPartLibrary->LoadBlueprintAssetDataFromPath(RootShipPartPath);
	UE_LOG(LogTemp, Log, TEXT("Loaded %d ship parts"), NumLoaded);

	ShipPartData.Reserve(NumLoaded);

	ShipPartLibrary->GetAssetDataList(ShipPartAssetData);
	for (const FAssetData& Data : ShipPartAssetData)
	{
		FShipPartData PartData{};
		PartData.Name = Data.AssetName; // TODO: strip prefix from name
		// TODO: figure out how to get the category (maybe through the path?)
		ShipPartData.Add(PartData);
	}

	bAssetDataLoaded = true;
}

AShipPart* UShipPartFactory::MakeShipPart(UObject* WorldContext, FName PartName)
{
	FString GeneratedClassName;
	for (FAssetData& Data : ShipPartAssetData)
	{
		if (Data.AssetName == PartName)
		{
			// We need the generated class name as it's the fully qualified name for the part.
			Data.GetTagValue("GeneratedClass", GeneratedClassName);
			break;
		}
	}

	if (GeneratedClassName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get generated class name for part: %s"), *PartName.ToString());
		return nullptr;
	}

	// TODO: cache class?
	UClass* PartClass = LoadClass<AShipPart>(nullptr, *GeneratedClassName);
	if (!PartClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load class for part: %s"), *PartName.ToString());
		return nullptr;
	}

	UWorld* WorldRef = GEngine->GetWorldFromContextObject(WorldContext);
	if (!ensureMsgf(WorldRef, TEXT("World is invalid")))
		return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = WorldRef->GetFirstPlayerController();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AShipPart* ShipPart = WorldRef->SpawnActor<AShipPart>(PartClass, FVector(0.f, 0.f, 300.f), FRotator::ZeroRotator, SpawnParams);
	if (!ShipPart)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create ship part from class: %s"), *GetNameSafe(PartClass));
		return ShipPart;
	}

	return ShipPart;
}