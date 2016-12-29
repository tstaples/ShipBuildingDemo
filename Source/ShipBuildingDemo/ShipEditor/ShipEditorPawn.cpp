// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPawn.h"


AShipEditorPawn::AShipEditorPawn()
{
	// TODO: handle movement bindings ourselves. Maybe make them toggle-able or something.
	bAddDefaultMovementBindings = false;
}

void AShipEditorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);
}