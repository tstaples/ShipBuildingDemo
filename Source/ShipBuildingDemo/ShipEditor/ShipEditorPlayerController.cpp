// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPlayerController.h"
#include "ShipPart.h"


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
			CurrentlyHeldShipPart->Select();
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
		//UE_LOG(LogTemp, Log, TEXT("Moving slected object to %s"), *WorldLocation.ToString());
		// TODO: do snapping
		
		// Translate the part by the mouse movement.
		const float Dist = FVector::Dist(GetPawn()->GetActorLocation(), CurrentlyHeldShipPart->GetActorLocation());
		FVector NewPosition = WorldLocation + WorldDirection * Dist;
		NewPosition.X = CurrentlyHeldShipPart->GetActorLocation().X; // TODO: use look axis (when movement enabled) to determine which to reset
		CurrentlyHeldShipPart->SetActorLocation(NewPosition);
	}
}
