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
	
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* CameraBoomComponent;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* CameraComponent;

protected:
	// Min distance the camera can be from the center point.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxZoomInDistance = 250.f;

	// Max distance the camera can be from the center point.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxZoomOutDistance = 2000.f;

	// Minimum distance the camera can be from the ground.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MinCameraHeight = 50.f;

	// Maximum distance the camera can be from the ground.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxCameraHeight = 1000.f;

	// How many units the camera moves per scroll tick.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float ZoomScrollModifier = 25.f;

	// How many units the camera moves per scroll tick.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float CameraVerticalPanScrollModifier = 25.f;

public:
	AShipEditorPawn();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
	virtual void AddControllerYawInput(float Val) override;
	virtual void AddControllerPitchInput(float Val) override;
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void MoveUp_World(float Val) override;

private:
	void OnZoom(float Val);
	void OnCameraVerticalPan(float val);
};
