// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "ShipEditorPawn.generated.h"

/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API AShipEditorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
	
public:
	AShipEditorPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	
};
