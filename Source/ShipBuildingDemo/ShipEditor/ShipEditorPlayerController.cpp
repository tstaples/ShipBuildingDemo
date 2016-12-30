// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPlayerController.h"
#include "ShipPart.h"
#include "ShipAttachPoint.h"


namespace
{
	// Clears an array but retains the memory.
	template<typename T>
	void ClearArray(TArray<T>& Arr, bool bRetainSlack = true)
	{
		const int32 Slack = bRetainSlack ? Arr.Num() : 0;
		Arr.Empty(Slack);
	}
}


AShipEditorPlayerController::AShipEditorPlayerController()
	: CurrentlyHeldShipPart(nullptr)
{
	bShowMouseCursor = true;
}

void AShipEditorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindAction("Select", IE_Pressed, this, &AShipEditorPlayerController::OnClick);
	InputComponent->BindAction("Select", IE_Released, this, &AShipEditorPlayerController::OnReleaseClick);
}

void AShipEditorPlayerController::OnClick()
{
	checkf(!HoldingShipPart(), TEXT("Can't click when already holding a ship part"));

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, true, Hit))
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
		ClearArray(CachedCompatiblePoints);
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
		// TODO: handle detaching when two attached parts are moved more than some distance from each other.

		// Get the movement delta from the mouse movement
		const FVector PreviousHeldPartLocation = CurrentlyHeldShipPart->GetActorLocation();
		const float Dist = FVector::Dist(GetPawn()->GetActorLocation(), PreviousHeldPartLocation);
		FVector NewPosition = WorldLocation + WorldDirection * Dist;
		NewPosition.X = PreviousHeldPartLocation.X; // TODO: use look axis (when movement enabled) to determine which to reset
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

AShipPart* AShipEditorPlayerController::CreateShipPart(TSubclassOf<AShipPart> PartClass)
{
	if (!ensureMsgf(GetWorld(), TEXT("World is invalid")))
		return nullptr;

	if (PartClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot create ship part from null class"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AShipPart* ShipPart = GetWorld()->SpawnActor<AShipPart>(PartClass, SpawnParams);
	if (!ShipPart)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create ship part from class: %s"), *GetNameSafe(PartClass));
		return ShipPart;
	}
	
	// Add the part to our internal list.
	ShipParts.Add(ShipPart);
	return ShipPart;
}

// NOTE: this will have to be re-calculated if we allow rotating parts.
bool AShipEditorPlayerController::CollectCompatiblePoints(const AShipPart* ShipPart, TArray<FAttachPointCacheEntry>& OutCompatiblePoints) const
{
	check(ShipPart);
	
	ClearArray(OutCompatiblePoints);

	// TODO: allow slight difference and compensate by rotating when snapping.
	static const float AllowedAngleDifference = 0.f; // Radians

	const auto& AttachPoints = ShipPart->GetAttachPoints();
	for (TActorIterator<AShipPart> It(GetWorld()); It; ++It)
	{
		AShipPart* OtherPart = *It;
		if (OtherPart == ShipPart)
			continue;

		auto AvailableAttachPoints = OtherPart->GetAvailableAttachPoints();
		for (auto* OtherPoint : AvailableAttachPoints)
		{
			for (auto* AttachPoint : AttachPoints)
			{
				// If the normals aren't within the allowed range then ignore them.
				const float Dot = FVector::DotProduct(AttachPoint->GetNormal(), OtherPoint->GetNormal());
				if (!FMath::IsNearlyEqual(Dot, -1.f, THRESH_NORMALS_ARE_PARALLEL))
					continue;

				// Check if they share any of the same allowed part types.
				if (!AttachPoint->IsCompatibleWith(OtherPoint))
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