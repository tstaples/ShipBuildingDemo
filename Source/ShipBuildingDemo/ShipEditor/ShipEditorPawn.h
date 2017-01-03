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
	
	class AShipEditorPlayerController* PCRef;
	
public:
	AShipEditorPawn();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void AddControllerYawInput(float Val) override;
	virtual void AddControllerPitchInput(float Val) override;
};
