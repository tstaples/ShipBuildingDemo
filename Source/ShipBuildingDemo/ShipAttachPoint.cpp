// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipAttachPoint.h"
#include "ShipPart.h"

void UShipAttachPoint::AttachPoints(UShipAttachPoint* A, UShipAttachPoint* B)
{
	A->AttachToPoint(B);
	B->AttachToPoint(A);
}

void UShipAttachPoint::DetachPoints(UShipAttachPoint* A, UShipAttachPoint* B)
{
	if (A->IsAttachedToPoint(B))
	{
		check(B->IsAttachedToPoint(A));
		A->DetachFromPoint();
		B->DetachFromPoint();
	}
}

//////////////////////////////////////////////////////////////////////////

// Sets default values for this component's properties
UShipAttachPoint::UShipAttachPoint()
	: OwningShipPart(nullptr)
	, bIsHighlighted(false)
	, AttachedToPoint(nullptr)
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Normal"));
	DirectionArrow->SetupAttachment(this);
	DirectionArrow->bHiddenInGame = false;
	DirectionArrow->ArrowSize = 0.75f;

	AttachPointSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachSphere"));
	AttachPointSphere->SetupAttachment(this);
	AttachPointSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachPointSphere->bHiddenInGame = false;

	// Load the sphere mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		AttachPointSphere->SetStaticMesh(SphereVisualAsset.Object);
		AttachPointSphere->SetWorldScale3D({ 0.25f, 0.25f, 0.25f });
	}

	// Load the material instance for the sphere.
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> SphereMaterialAsset(TEXT("/Game/Materials/MATINST_AttachPointNode.MATINST_AttachPointNode"));
	if (SphereMaterialAsset.Succeeded())
	{
		AttachPointSphere->SetMaterial(0, SphereMaterialAsset.Object);
	}
}

void UShipAttachPoint::InitializeComponent()
{
	Super::InitializeComponent();
}

void UShipAttachPoint::PostInitProperties()
{
	Super::PostInitProperties();

	// Use owner defaults if we have no compatible parts set.
	// TODO: Move this somewhere that the changes get reflected in the editor.
	OwningShipPart = Cast<AShipPart>(GetOwner());
	if (OwningShipPart && CompatibleParts.Num() == 0)
	{
		CompatibleParts = OwningShipPart->GetDefaultCompatibleParts();
	}
	else if (OwningShipPart == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No owner"));
	}
}

void UShipAttachPoint::BeginPlay()
{
	Super::BeginPlay();	

	// Create a DMI of the sphere material so we can dynamically change the color param.
	// Doing this in beginplay to avoid the material disappearing when hot-reloading.
	if (!SphereDMI)
	{
		SphereDMI = AttachPointSphere->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void UShipAttachPoint::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

bool UShipAttachPoint::IsCompatibleWith(EPartType PartType) const
{
	return CompatibleParts.Contains(PartType);
}

bool UShipAttachPoint::IsAttached() const
{
	return (AttachedToPoint != nullptr);
}

void UShipAttachPoint::AttachToPoint(UShipAttachPoint* PointToAttachTo)
{
	checkf(!IsAttached(), TEXT("Already attached to another point. Must detach first."));
	AttachedToPoint = PointToAttachTo;
	//UE_LOG(LogTemp, Log, TEXT("%s Attached to %s"), *GetNameSafe(this), *GetNameSafe(AttachedToPoint));

	// Hide highlight, sphere and arrow once we've been attached to something.
	if (bIsHighlighted)
	{
		SetHighlighted(false);
		DirectionArrow->SetVisibility(false);
		AttachPointSphere->SetVisibility(false);
	}
}

void UShipAttachPoint::DetachFromPoint()
{
	//UE_LOG(LogTemp, Log, TEXT("%s Detatched from %s"), *GetNameSafe(this), *GetNameSafe(AttachedToPoint));
	AttachedToPoint = nullptr;

	// Show them again once we're free.
	DirectionArrow->SetVisibility(true);
	AttachPointSphere->SetVisibility(true);
}

bool UShipAttachPoint::IsAttachedToPoint(const UShipAttachPoint* OtherPoint) const
{
	return (AttachedToPoint != nullptr && AttachedToPoint == OtherPoint);
}

FVector UShipAttachPoint::GetNormal() const
{
	checkf(GetComponentRotation() == DirectionArrow->GetComponentRotation(), L"Comp and arrow rots don't match");
	return GetComponentRotation().Vector();
}

void UShipAttachPoint::SetHighlighted(bool bHighlighted)
{
	if (bHighlighted == bIsHighlighted || !ensureMsgf(SphereDMI != nullptr, L"SphereDMI not set"))
		return;

	// Change the color param to green or white depending on if we're highlighting or not.
	// 'Color' param is exposed via the material instance.
	const FLinearColor NewColor = bHighlighted ? FLinearColor::Green : FLinearColor::White;
	SphereDMI->SetVectorParameterValue(TEXT("Color"), NewColor);

	bIsHighlighted = bHighlighted;
}
