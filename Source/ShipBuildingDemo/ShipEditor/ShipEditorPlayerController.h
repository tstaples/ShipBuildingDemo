// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ShipEditorPlayerController.generated.h"

class AShipPart;

/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API AShipEditorPlayerController : public APlayerController
{
	GENERATED_BODY()

	// Ship part currently being held.
	UPROPERTY(Transient)
	AShipPart* CurrentlyHeldShipPart;

	// All of the ship parts.
	//UPROPERTY()
	//TArray<AShipPart*> ShipParts;

public:
	AShipEditorPlayerController();
	
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE bool HoldingShipPart() const { return (CurrentlyHeldShipPart != nullptr); }

private:
	UFUNCTION()
	void OnClick();
	UFUNCTION()
	void OnReleaseClick();
};
