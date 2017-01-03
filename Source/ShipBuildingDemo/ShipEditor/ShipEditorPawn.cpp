// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPawn.h"
#include "ShipEditor/ShipEditorPlayerController.h"


AShipEditorPawn::AShipEditorPawn()
{
	// TODO: handle movement bindings ourselves. Maybe make them toggle-able or something.
	bAddDefaultMovementBindings = true;
}

void AShipEditorPawn::BeginPlay()
{
	Super::BeginPlay();

	PCRef = Cast<AShipEditorPlayerController>(GetController());
	check(PCRef);
}

void AShipEditorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);
}

void AShipEditorPawn::AddControllerYawInput(float Val)
{
	// Only allow turning when we're not holding anything
	if (PCRef && !PCRef->HoldingShipPart())
	{
		Super::AddControllerYawInput(Val);
	}
}

void AShipEditorPawn::AddControllerPitchInput(float Val)
{
	if (PCRef && !PCRef->HoldingShipPart())
	{
		Super::AddControllerPitchInput(Val);
	}
}