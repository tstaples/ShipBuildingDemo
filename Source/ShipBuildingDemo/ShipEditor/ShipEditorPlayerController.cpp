// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPlayerController.h"
#include "ShipBuilding/ShipPart.h"
#include "ShipBuilding/ShipAttachPoint.h"
#include "Serialization/ShipSaveGame.h"
#include "ShipBuilding/ShipPartFactory.h"


AShipEditorPlayerController::AShipEditorPlayerController()
	: CurrentlyHeldShipPart(nullptr)
	, ShipPartFactory(nullptr)
{
	bShowMouseCursor = true;
}

void AShipEditorPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ShipPartFactory = NewObject<UShipPartFactory>();
	ShipPartFactory->Init("/Game/ShipParts");
}

void AShipEditorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindAction("Select", IE_Pressed, this, &AShipEditorPlayerController::OnClick);
	InputComponent->BindAction("Select", IE_Released, this, &AShipEditorPlayerController::OnReleaseClick);
	InputComponent->BindAction("Delete", IE_Pressed, this, &AShipEditorPlayerController::DeleteSelectedPart);
}

void AShipEditorPlayerController::OnClick()
{
	checkf(!HoldingShipPart(), TEXT("Can't click when already holding a ship part"));

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit))
		return;

	if (Hit.Actor.IsValid() && Hit.Actor->IsA<AShipPart>())
	{
		UE_LOG(LogTemp, Log, TEXT("Clicked ship part: %s"), *GetNameSafe(Hit.GetActor()));

		CurrentlyHeldShipPart = Cast<AShipPart>(Hit.GetActor());
		if (ensure(CurrentlyHeldShipPart != nullptr))
		{
			// TODO: remove this if we end up removing all the logic anyway.
			CurrentlyHeldShipPart->Select();

			// Collect and store all points compatible with the currently held one so we don't have to re-lookup every tick.
			CollectCompatiblePoints(CurrentlyHeldShipPart, CachedCompatiblePoints);
			SetCachedPointsHighlighted(true, CachedCompatiblePoints);
		}
	}
}

void AShipEditorPlayerController::OnReleaseClick()
{
	if (HoldingShipPart())
	{
		UE_LOG(LogTemp, Log, TEXT("Released ship part: %s"), *GetNameSafe(CurrentlyHeldShipPart));
		CurrentlyHeldShipPart->Deselect();
		CurrentlyHeldShipPart = nullptr;

		SetCachedPointsHighlighted(false, CachedCompatiblePoints);

		// Clear the cache but keep the memory allocated.
		// TODO: track the last selected part and re-use the cache if we select the same one we just did.
		// Will involve invalidation when a new part is created though.
		ShipUtils::ClearArray(CachedCompatiblePoints);
	}
}

void AShipEditorPlayerController::DeleteSelectedPart()
{
	if (CurrentlyHeldShipPart)
	{
		UE_LOG(LogTemp, Log, TEXT("Destroying ship part: %s"), *GetNameSafe(CurrentlyHeldShipPart));
		DestroyShipPart(CurrentlyHeldShipPart);
		OnReleaseClick();
	}
}

void AShipEditorPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HoldingShipPart())
		return;

	FVector WorldLocation(ForceInitToZero);
	FVector WorldDirection(ForceInitToZero);
	if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		// TODO: Offset by mouse position so the object doesn't snap so the mouse is in the center.

		// Get the movement delta from the mouse movement.
		// World location is always ~10 units from the pawn's position, so we must offset by the part's distance or it will move to where we are.
		const FVector PreviousHeldPartLocation = CurrentlyHeldShipPart->GetActorLocation();
		const float Dist = FVector::Dist(WorldLocation, PreviousHeldPartLocation);
		FVector NewPosition = WorldLocation + WorldDirection * Dist;
		const FVector Delta = NewPosition - PreviousHeldPartLocation;

		if (CurrentlyHeldShipPart->IsAttached())
		{
			// Mouse hasn't moved far enough to un-snap.
			if (Delta.Size() < CurrentlyHeldShipPart->GetMinSnapDistance())
				return;

			// Detach the current ship part
			CurrentlyHeldShipPart->DetatchAllPoints();

			// Re-compute
			CollectCompatiblePoints(CurrentlyHeldShipPart, CachedCompatiblePoints);
			SetCachedPointsHighlighted(true, CachedCompatiblePoints);
		}

		// Search the cache for any entry whose two points meet the requirements to snap together.
		const int32 CacheIndex = FindPointsToSnapTogether(CachedCompatiblePoints, Delta);
		if (CacheIndex != INDEX_NONE)
		{
			// Offset the ship part by the delta of the attach points.
			FAttachPointCacheEntry& BestEntry = CachedCompatiblePoints[CacheIndex];
			UShipAttachPoint* OwnedPoint = BestEntry.OwnedPoint;
			UShipAttachPoint* OtherPoint = BestEntry.OtherPoint;
			// TODO: use surface position rather than component point so they don't need to be positioned perfectly.
			const FVector PointDelta = OtherPoint->GetComponentLocation() - OwnedPoint->GetComponentLocation();
			if (!PointDelta.IsNearlyZero()) // This shouldn't be an issue.
			{
				NewPosition = CurrentlyHeldShipPart->GetActorLocation() + PointDelta;

				//UE_LOG(LogTemp, Log, TEXT("Attached %s and %s"), *GetNameSafe(OwnedPoint), *GetNameSafe(OtherPoint));
				UShipAttachPoint::AttachPoints(OwnedPoint, OtherPoint);
			}
		}

		CurrentlyHeldShipPart->SetActorLocation(NewPosition);
	}
}

void AShipEditorPlayerController::SpawnShipPart(FName PartName)
{
	AShipPart* ShipPart = ShipPartFactory->MakeShipPart(this, PartName);
	if (ShipPart)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully created part: %s"), *PartName.ToString());
		// Add the part to our internal list.
		ShipParts.Add(ShipPart);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to make part: %s"), *PartName.ToString());
	}
}

// NOTE: this will have to be re-calculated if we allow rotating parts.
bool AShipEditorPlayerController::CollectCompatiblePoints(const AShipPart* ShipPart, TArray<FAttachPointCacheEntry>& OutCompatiblePoints) const
{
	check(ShipPart);
	
	ShipUtils::ClearArray(OutCompatiblePoints);

	// TODO: allow slight difference and compensate by rotating when snapping.
	// Will be needed for snapping parts onto non-flat surfaces.
	static const float AllowedAngleDifference = 0.f; // Radians

	// Grab the attach points for the selected part to avoid fetching inside the loop.
	const EPartType SelectedPartType = ShipPart->GetPartType();
	const auto& AttachPoints = ShipPart->GetAttachPoints();
	if (AttachPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ship part %s has no attach points."), *GetNameSafe(ShipPart));
		return false;
	}

	// Go through all the ship parts.
	// TODO: ensure the ship parts array is correctly populated when ships are loaded in when that's implemented.
	for (AShipPart* OtherPart : ShipParts)
	{
		// Ignore the part we're checking
		if (OtherPart == ShipPart)
			continue;

		// Does the other part have any points that are compatible with the selected part?
		const EPartType OtherPartType = OtherPart->GetPartType();
		auto PointsCompatibleWithSelected = OtherPart->GetPointsCompatibleWith(SelectedPartType);
		if (PointsCompatibleWithSelected.Num() == 0)
			continue;

		// Check if the selected part has any points that are compatible with the other part.
		for (auto* AttachPoint : AttachPoints)
		{
			if (!AttachPoint->IsCompatibleWith(OtherPartType))
				continue;

			// Iter through all the compatible points on the other part
			for (auto* OtherPoint : PointsCompatibleWithSelected)
			{
				// TODO: test points against all filters defined by the selected part if we need that kind of granularity.
				// If the normals aren't within the allowed range then ignore them.
				const float Dot = FVector::DotProduct(AttachPoint->GetNormal(), OtherPoint->GetNormal());
				if (!FMath::IsNearlyEqual(Dot, -1.f, THRESH_NORMALS_ARE_PARALLEL))
					continue;

				OutCompatiblePoints.Add({ AttachPoint, OtherPoint });
			}
		}
	}
	return (OutCompatiblePoints.Num() > 0);
}

void AShipEditorPlayerController::SetCachedPointsHighlighted(bool bHighlighted, TArray<FAttachPointCacheEntry>& InPoints) const
{
	for (auto& Entry : InPoints)
	{
		check(Entry.IsValid());
		Entry.OwnedPoint->SetHighlighted(bHighlighted);
		Entry.OtherPoint->SetHighlighted(bHighlighted);
	}
}

int32 AShipEditorPlayerController::FindPointsToSnapTogether(const TArray<FAttachPointCacheEntry>& CompatiblePoints, const FVector& Delta) const
{
	if (CachedCompatiblePoints.Num() == 0)
		return INDEX_NONE;

	// Cache these outside the loop as they won't change per entry.
	const AShipPart* ShipPart = CompatiblePoints[0].OwnedPoint->GetOwningShipPart();
	const FBoxSphereBounds HeldSnapBounds = ShipPart->GetSnapBounds();

	int32 BestIndex = INDEX_NONE;
	float BestDistSq = FLT_MAX;
	for (int32 i = 0; i < CompatiblePoints.Num(); ++i)
	{
		const auto& CacheEntry = CompatiblePoints[i];
		const UShipAttachPoint* OwnedPoint = CacheEntry.OwnedPoint;
		const UShipAttachPoint* OtherPoint = CacheEntry.OtherPoint;
		const AShipPart* OtherPart = OtherPoint->GetOwningShipPart();
		checkf(OwnedPoint != OtherPoint, TEXT("OwnedPoint and OtherPoint should not be the same."));
		checkf(ShipPart != OtherPart, TEXT("ShipPart should not be linked to itself in cache."));

		// TODO: offset owned points by delta
		// TODO: check delta is in direction of cached point/part. (Probably only needed for super small pieces maybe).

		// Broad phase check.
		if (!FBoxSphereBounds::BoxesIntersect(HeldSnapBounds, OtherPart->GetSnapBounds()))
			continue;

		// Find the two nodes that are closest together and within the min snap distance.
		const float MinSnapDistanceSq = FMath::Square(ShipPart->GetMinSnapDistance());
		const float DistSq = FVector::DistSquared(OwnedPoint->GetComponentLocation(), OtherPoint->GetComponentLocation());
		if (DistSq <= MinSnapDistanceSq && DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestIndex = i;
		}
	}

	return BestIndex;
}

void AShipEditorPlayerController::DestroyShipPart(AShipPart* ShipPart)
{
	// Ensure both as either raise concerns
	if (!ensure(ShipPart) || !ensure(!ShipPart->IsActorBeingDestroyed()))
		return;

	ShipParts.Remove(CurrentlyHeldShipPart);
	ShipPart->DetatchAllPoints();
	ShipPart->Destroy();
}

void AShipEditorPlayerController::ClearShip()
{
	UE_LOG(LogTemp, Log, TEXT("Clearing ship"));
	ShipUtils::DestroyActorArray(ShipParts);
}

//////////////////////////////////////////////////////////////////////////
// Saving
//////////////////////////////////////////////////////////////////////////

bool AShipEditorPlayerController::SaveShip(const FString& ShipName)
{
	UE_LOG(LogTemp, Log, TEXT("Saving ship: %s"), *ShipName);

	UShipSaveGame* ShipSaveData = GetSaveDataForShip(ShipName);
	if (!ShipSaveData)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get or create save data for ship: %s"), *ShipName);
		return false;
	}

	if (!ShipSaveData->SaveShip(ShipName, ShipParts))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save data for ship: %s"), *ShipName);
		return false;
	}

	// TODO: update some internal flag that there are no unsaved changes (set false when change is made). Used to check if we should prompt to save before loading/exiting.
	// TODO: maybe create some sort of prefix for the slot name.
	return UGameplayStatics::SaveGameToSlot(ShipSaveData, ShipName, 0);
}

bool AShipEditorPlayerController::LoadShip(const FString& ShipName)
{
	UE_LOG(LogTemp, Log, TEXT("Loading ship: %s"), *ShipName);

	// TODO: check if we're loading the one we're currently using.
	// TODO: prompt to save current ship if we already have one loaded.
	if (ShipParts.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing ship parts exist; they will be destroyed when loading %s (for now)"), *ShipName);
		ShipUtils::DestroyActorArray(ShipParts, true);
	}

	if (!UGameplayStatics::DoesSaveGameExist(ShipName, 0))
	{
		UE_LOG(LogTemp, Error, TEXT("No save data exists for ship: %s"), *ShipName);
		return false;
	}

	UShipSaveGame* ShipSaveData = Cast<UShipSaveGame>(UGameplayStatics::LoadGameFromSlot(ShipName, 0));
	if (!ShipSaveData)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load save data for ship: %s"), *ShipName);
		return false;
	}
	checkf(ShipSaveData->GetShipName() == ShipName, TEXT("Ship name in record does not match the one requested to be loaded."));

	// Convert records to ship parts and store in our ShipParts array.
	if (!ShipSaveData->LoadShip(this, ShipParts))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create ship parts from save data for ship: %s"), *ShipName);
		return false;
	}

	return true;
}

UShipSaveGame* AShipEditorPlayerController::GetSaveDataForShip(const FString& ShipName) const
{
	UShipSaveGame* ShipSaveData = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(ShipName, 0))
	{
		ShipSaveData = Cast<UShipSaveGame>(UGameplayStatics::LoadGameFromSlot(ShipName, 0));
	}
	else
	{
		ShipSaveData = Cast<UShipSaveGame>(UGameplayStatics::CreateSaveGameObject(UShipSaveGame::StaticClass()));
	}
	return ShipSaveData;
}

bool AShipEditorPlayerController::GetSavedShipNames(TArray<FName>& OutShipNames)
{
	// Directory visitor implementation that collects the filenames from a directory and optionally formats them.
	struct FSaveGameVisitor : public IPlatformFile::FDirectoryVisitor
	{
		TArray<FName>& SaveGameFileNames;
		TFunction<void(FString& Filename)> FormatFunction;

		FSaveGameVisitor(TArray<FName>& OutFileNames, const TFunction<void(FString& Filename)>& Formatter) 
			: SaveGameFileNames(OutFileNames)
			, FormatFunction(Formatter) {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (!bIsDirectory)
			{
				FString Filename{ FilenameOrDirectory };
				FormatFunction(Filename);
				SaveGameFileNames.Add(*Filename);
			}
			return true;
		}
	};

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString SaveGameDir = FString::Printf(TEXT("%s/SaveGames"), *FPaths::GameSavedDir());

	FSaveGameVisitor SaveGameVisitor{ OutShipNames, [](FString& Filename) {
		Filename = FPaths::GetBaseFilename(Filename); // Get just the filename.
	} };
	return PlatformFile.IterateDirectory(*SaveGameDir, SaveGameVisitor);
}