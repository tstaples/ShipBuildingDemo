// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipSaveGame.h"
#include "ShipBuilding/ShipPart.h"


//////////////////////////////////////////////////////////////////////////

FShipSaveGameArchiveProxy::FShipSaveGameArchiveProxy(FArchive& innerArchive)
	: FObjectAndNameAsStringProxyArchive(innerArchive, true) 
{
	ArIsSaveGame = true;
}

//////////////////////////////////////////////////////////////////////////

bool UShipSaveGame::SaveShip(const FString& NameOfShip, const TArray<AShipPart*>& ShipParts)
{
	// Clear any existing records but retain memory for the number of parts we'll be adding records for.
	ShipPartRecords.Empty(ShipParts.Num());

	ShipName = NameOfShip;

	for (AShipPart* ShipPart : ShipParts)
	{
		FShipPartRecord Record{};
		Record.PartTransform = ShipPart->GetActorTransform();
		Record.ShipTemplateName = ShipPart->GetClass()->GetPathName();

		FMemoryWriter MemoryWriter{ Record.ShipPartData };

		// simple proxy specifies the serialisation behaviour
		FShipSaveGameArchiveProxy Archive{ MemoryWriter };
		
		ShipPart->Serialize(Archive);
		
		ShipPartRecords.Add(MoveTemp(Record));
	}
	return true;
}

bool UShipSaveGame::LoadShip(UObject* WorldContext, TArray<AShipPart*>& OutShipParts) const
{
	UWorld* WorldRef = GEngine->GetWorldFromContextObject(WorldContext);
	check(WorldRef);

	// TODO: should we allow saving no parts?
	if (ShipPartRecords.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No ship records to load for %s"), *ShipName);
		return true;
	}

	// This should be empty. TODO: be more explicit here when not tired.
	if (OutShipParts.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OutShipParts already contains data; there are likely undestroyed ship parts."));
		OutShipParts.Empty(ShipPartRecords.Num());
	}

	// Create the ship part instances from the records and store in OutShipParts.
	for (const FShipPartRecord& Record : ShipPartRecords)
	{
		UClass* ShipTemplate = FindObject<UClass>(ANY_PACKAGE, *Record.ShipTemplateName);
		if (!ShipTemplate)
		{
			ShipTemplate = LoadObject<UClass>(NULL, *Record.ShipTemplateName);
		}

		auto ShipPart = Cast<AShipPart>(UGameplayStatics::BeginDeferredActorSpawnFromClass(WorldRef, ShipTemplate, Record.PartTransform));
		if (!ShipPart) 
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create ship part: %s"), ShipTemplate);
			return false;
		}

		FMemoryReader MemoryReader{ Record.ShipPartData, true };
		FShipSaveGameArchiveProxy Archive{ MemoryReader };

		ShipPart->Serialize(Archive);

		ShipPart = Cast<AShipPart>(UGameplayStatics::FinishSpawningActor(ShipPart, Record.PartTransform));
		check(ShipPart);
		OutShipParts.Add(MoveTemp(ShipPart));
	}

	return true;
}