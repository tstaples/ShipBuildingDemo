// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipPartFactory.h"
#include "ShipPart.h"

DECLARE_LOG_CATEGORY_CLASS(LogShipPartFactory, Log, All);

namespace
{
	// Gets the directory name of a file.
	static FString DirName(const FString& InPath)
	{
		FString Path, Filename, Extension;
		FPaths::Split(InPath, Path, Filename, Extension);
		return Path;
	}

	static FString StripPrefix(const FString& Prefix, const FString& InString, ESearchCase::Type = ESearchCase::IgnoreCase)
	{
		FString Temp {InString};
		Temp.RemoveFromStart(Prefix);
		return Temp;
	}
}

//////////////////////////////////////////////////////////////////////////
// FShipPartData
//////////////////////////////////////////////////////////////////////////

const FString FShipPartData::Prefix = "BLU_";

FShipPartData::FShipPartData(const FAssetData& Data)
: AssetData(Data)
{
	FullName = Data.AssetName;

	// Verify asset name
	if (!FullName.ToString().StartsWith(Prefix))
	{
		UE_LOG(LogShipPartFactory, Warning, TEXT("Asset name for ship part %s does not contain the prefix: %s."), *FullName.ToString(), *Prefix);
	}
}

FString FShipPartData::GetGeneratedClassName(const FShipPartData& ShipPartData)
{
	// We need the generated class name as it's the fully qualified name for the part.
	// This is the only way I know of to get the actual class of a blueprint to spawn it since Data.GetClass() just returns UBlueprint.
	FString GeneratedClassName;
	ShipPartData.AssetData.GetTagValue("GeneratedClass", GeneratedClassName);
	return GeneratedClassName;
}

//////////////////////////////////////////////////////////////////////////

UShipPartFactory::UShipPartFactory()
: bAssetDataLoaded(false)
{
}

void UShipPartFactory::Init(const FString& RootShipPartPath)
{
	if (bAssetDataLoaded)
	{
		return;
	}

	// Create an object library for the ship parts and load the asset data for all of them.
	ShipPartLibrary = UObjectLibrary::CreateLibrary(AShipPart::StaticClass(), true, false);
	const int32 NumLoaded = ShipPartLibrary->LoadBlueprintAssetDataFromPath(RootShipPartPath);
	if (NumLoaded == 0)
	{
		UE_LOG(LogShipPartFactory, Warning, TEXT("No ship parts were loaded."));
		return;
	}

	const TMap<FString, EPartType> ShipPartPaths = MakeShipPartPathsToTypes(RootShipPartPath);

	TArray<FAssetData> ShipPartAssetData;
	ShipPartLibrary->GetAssetDataList(ShipPartAssetData);
	ShipPartData.Reserve(NumLoaded);
	for (const FAssetData& Data : ShipPartAssetData)
	{
		FShipPartData PartData{ Data };
		PartData.Name = *StripPrefix(FShipPartData::Prefix, Data.AssetName.ToString());

		// Use the path to lookup the type
		const FString Path = DirName(Data.ObjectPath.ToString());
		PartData.PartType = ShipPartPaths.Contains(Path) ? ShipPartPaths[Path] : EPartType::PT_MAX;

		ShipPartData.Add(MoveTemp(PartData));
	}

	bAssetDataLoaded = true;
}

AShipPart* UShipPartFactory::MakeShipPart(UObject* WorldContext, FName PartName, FVector SpawnLocation /*= FVector(0.f, 0.f, 30.f)*/)
{
	checkf(HasLoadedAssetData(), TEXT("Asset data has not been loaded, ensure that Init() has been called first."));

	const FShipPartData* Data = ShipPartData.FindByPredicate([&PartName](auto&& Data) { return Data.Name == PartName; });
	if (!Data)
	{
		UE_LOG(LogShipPartFactory, Error, TEXT("Failed to find data for part: %s"), *PartName.ToString());
		return nullptr;
	}

	const FString GeneratedClassName = FShipPartData::GetGeneratedClassName(*Data);
	if (GeneratedClassName.IsEmpty())
	{
		UE_LOG(LogShipPartFactory, Error, TEXT("Failed to get generated class name for part: %s"), *PartName.ToString());
		return nullptr;
	}

	// TODO: cache class?
	UClass* PartClass = LoadClass<AShipPart>(nullptr, *GeneratedClassName);
	if (!PartClass)
	{
		UE_LOG(LogShipPartFactory, Error, TEXT("Failed to load class for part: %s"), *PartName.ToString());
		return nullptr;
	}

	UWorld* WorldRef = GEngine->GetWorldFromContextObject(WorldContext);
	if (!ensureMsgf(WorldRef, TEXT("World is invalid")))
	{
		return nullptr;
	}
	
	// TODO: Maybe spawn the part where the mouse is (drag and drop)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = WorldRef->GetFirstPlayerController();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AShipPart* ShipPart = WorldRef->SpawnActor<AShipPart>(PartClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!ShipPart)
	{
		UE_LOG(LogShipPartFactory, Error, TEXT("Failed to create ship part from class: %s"), *GetNameSafe(PartClass));
		return ShipPart;
	}

	return ShipPart;
}

TMap<FString, EPartType> UShipPartFactory::MakeShipPartPathsToTypes(const FString& RootPath) const
{
	TMap<FString, EPartType> PathToTypes;

	static const FString EnumPrefix{ "PT_" };
	static const FString EnumTypeName{ "EPartType" };

	for (int32 i = 0; i < (int32)EPartType::PT_MAX; ++i)
	{
		const EPartType PartType = EPartType(i);
		FString EnumName = ShipUtils::EnumValueToString(EnumTypeName, PartType);
		if (EnumName.Len() == 0)
		{
			UE_LOG(LogShipPartFactory, Error, TEXT("Failed to get name for enum value: %d"), i);
			continue;
		}

		// Strip the prefix as the dir name does not contain it.
		EnumName.RemoveFromStart(EnumPrefix);
		
		// Make the path
		const FString PartPath = FPaths::Combine(*RootPath, *EnumName);
		PathToTypes.Add(MoveTemp(PartPath), PartType);
	}
	return PathToTypes;
}