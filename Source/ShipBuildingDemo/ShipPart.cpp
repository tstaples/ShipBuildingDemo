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
	//UE_LOG(LogTemp, Log, TEXT("%s Selected"), *GetNameSafe(this));
	//if (ShipPartMesh)
	//{
	//	// TODO: change opacity
	//}
}

void AShipPart::Deselect()
{
}

bool AShipPart::IsAttached() const
{
	return AttachPoints.ContainsByPredicate([](UShipAttachPoint* P) { return P->IsAttached(); });
}

void AShipPart::DetatchAllPoints()
{
	for (auto* AttachPoint : AttachPoints)
	{
		if (AttachPoint->IsAttached())
		{
			auto* AttachedTo = AttachPoint->GetAttachedToPoint();
			AttachPoint->DetachFromPoint();
			AttachedTo->DetachFromPoint();
		}
	}
}

void AShipPart::SetAllPointsHighlighted(bool bHighlighted)
{
	for (auto* AttachPoint : AttachPoints)
	{
		AttachPoint->SetHighlighted(bHighlighted);
	}
}

TArray<UShipAttachPoint*> AShipPart::GetPointsCompatibleWith(EPartType Type) const
{
	TArray<UShipAttachPoint*> Points;
	for (auto* AttachPoint : AttachPoints)
	{
		if (!AttachPoint->IsAttached() && AttachPoint->IsCompatibleWith(Type))
			Points.Add(AttachPoint);
	}
	return Points;
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
