// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipPart.h"
#include "ShipAttachPoint.h"


// Sets default values
AShipPart::AShipPart()
	: ShipPartMesh(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AShipPart::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ShipPartMesh = FindComponentByClass<UStaticMeshComponent>();

	// Store the attach point components.
	auto Components = GetComponentsByClass(UShipAttachPoint::StaticClass());
	AttachPoints.Reserve(Components.Num());
	for (auto* Component : Components)
	{
		AttachPoints.Add(Cast<UShipAttachPoint>(Component));
	}
}

void AShipPart::BeginPlay()
{
	Super::BeginPlay();
}

void AShipPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShipPart::Select()
{
	UE_LOG(LogTemp, Log, TEXT("%s Selected"), *GetNameSafe(this));
	//if (ShipPartMesh)
	//{
	//	// TODO: change opacity
	//}

	bool OtherPartsExist = false;
	for (TActorIterator<AShipPart> It(GetWorld()); It; ++It)
	{
		AShipPart* OtherPart = *It;
		if (OtherPart == this)
			continue;

		OtherPartsExist = true;
		auto AvailableAttachPoints = OtherPart->GetAvailableAttachPoints();
		for (auto* OtherPoint : AvailableAttachPoints)
		{
			for (auto* AttachPoint : AttachPoints)
			{
				if (AttachPoint->IsCompatibleWith(OtherPoint))
				{
					AttachPoint->SetHighlighted(true);
					OtherPoint->SetHighlighted(true);
				}
			}
		}
	}

	if (!OtherPartsExist)
	{
		for (auto* AttachPoint : AttachPoints)
		{
			AttachPoint->SetHighlighted(true);
		}
	}
}

void AShipPart::Deselect()
{
	UE_LOG(LogTemp, Log, TEXT("%s DeSelected"), *GetNameSafe(this));

	for (TActorIterator<AShipPart> It(GetWorld()); It; ++It)
	{
		(*It)->UnhighlightAllAttachPoints();
	}
}

void AShipPart::UnhighlightAllAttachPoints()
{
	for (auto* AttachPoint : AttachPoints)
	{
		AttachPoint->SetHighlighted(false);
	}
}

TArray<UShipAttachPoint*> AShipPart::GetAvailableAttachPoints() const
{
	TArray<UShipAttachPoint*> Points;
	for (auto* AttachPoint : AttachPoints)
	{
		if (!AttachPoint->IsAttached())
			Points.Add(AttachPoint);
	}
	return Points;
}
