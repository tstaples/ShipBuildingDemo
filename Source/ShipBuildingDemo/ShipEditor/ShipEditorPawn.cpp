// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipEditorPawn.h"
#include "ShipEditor/ShipEditorPlayerController.h"


AShipEditorPawn::AShipEditorPawn()
{
	// TODO: handle movement bindings ourselves. Maybe make them toggle-able or something.
	bAddDefaultMovementBindings = true;

	CameraBoomComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoomComponent->SetupAttachment(RootComponent);
	CameraBoomComponent->bDoCollisionTest = true; // Enable collision so the camera doesn't go through the floor
	CameraBoomComponent->bUsePawnControlRotation = true; // Let it actually rotate as user input rotates the controller
	CameraBoomComponent->bEnableCameraLag = true; // Smooth vertical movement
	CameraBoomComponent->bEnableCameraRotationLag = true; // Give the camera some weight
	CameraBoomComponent->TargetArmLength = 500.f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(CameraBoomComponent);
}

void AShipEditorPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
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

	InInputComponent->BindAxis("Zoom", this, &AShipEditorPawn::OnZoom);
}

void AShipEditorPawn::AddMovementInput(FVector WorldDirection, float ScaleValue /*= 1.0f*/, bool bForce /*= false*/)
{
	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
}

void AShipEditorPawn::AddControllerYawInput(float Val)
{
	if (Val == 0.f)
	{
		return;
	}

	// Only allow turning when we're not holding anything and the right mouse button is down.
	if (PCRef && !PCRef->HoldingShipPart() && 
		PCRef->GetInputAnalogKeyState(EKeys::RightMouseButton) == 1.f &&
		PCRef->GetInputAnalogKeyState(EKeys::LeftMouseButton) == 0.f)
	{
		Super::AddControllerYawInput(Val);
	}
}

void AShipEditorPawn::AddControllerPitchInput(float Val)
{
	if (Val == 0.f)
	{
		return;
	}

	// Only allow turning when we're not holding anything and the right mouse button is down.
	if (PCRef && !PCRef->HoldingShipPart() &&
		PCRef->GetInputAnalogKeyState(EKeys::RightMouseButton) == 1.f &&
		PCRef->GetInputAnalogKeyState(EKeys::LeftMouseButton) == 0.f)
	{
		Super::AddControllerPitchInput(Val);
	}
}

void AShipEditorPawn::MoveForward(float Val)
{
	// Do nothing
}

void AShipEditorPawn::MoveRight(float Val)
{
	// Do nothing
}

void AShipEditorPawn::MoveUp_World(float Val)
{
	// Do nothing
}

void AShipEditorPawn::OnZoom(float Val)
{
	// This is called every frame so ignore no input.
	if (Val == 0.f)
	{
		return;
	}

	// Only zoom while shift is held, otherwise pan. We can't add modifier keys to input axis bindings so this will do.
	if (PCRef->GetInputAnalogKeyState(EKeys::LeftShift) == 0.f)
	{
		OnCameraVerticalPan(Val);
		return;
	}

	const float Delta = Val * ZoomScrollModifier;
	CameraBoomComponent->TargetArmLength = FMath::Clamp(CameraBoomComponent->TargetArmLength - Delta, MaxZoomInDistance, MaxZoomOutDistance);
}

void AShipEditorPawn::OnCameraVerticalPan(float Val)
{
	const float Delta = Val * CameraVerticalPanScrollModifier;
	FVector Location = GetActorLocation();
	Location.Z = FMath::Clamp(Location.Z - Delta, MinCameraHeight, MaxCameraHeight);
	SetActorLocation(Location);
}