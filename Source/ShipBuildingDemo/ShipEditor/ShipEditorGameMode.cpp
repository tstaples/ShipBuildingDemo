// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorGameMode.h"
#include "ShipEditorPawn.h"
#include "ShipEditorPlayerController.h"
#include "ShipEditorHUD.h"


AShipEditorGameMode::AShipEditorGameMode()
{
	DefaultPawnClass = AShipEditorPawn::StaticClass();
	PlayerControllerClass = AShipEditorPlayerController::StaticClass();
	HUDClass = AShipEditorHUD::StaticClass();
}