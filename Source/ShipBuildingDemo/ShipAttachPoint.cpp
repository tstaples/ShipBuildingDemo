// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipBuildingDemo.h"
#include "ShipAttachPoint.h"
#include "ShipPart.h"

#include "Components/BillboardComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

// Sets default values for this component's properties
UShipAttachPoint::UShipAttachPoint()
	: OwningShipPart(nullptr)
	, bIsHighlighted(false)
	, AttachedToShipPart(nullptr)
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	AttachPointSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachSphere"));
	AttachPointSphere->SetupAttachment(this);
	AttachPointSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachPointSphere->bHiddenInGame = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		AttachPointSphere->SetStaticMesh(SphereVisualAsset.Object);
		AttachPointSphere->SetWorldScale3D({ 0.25f, 0.25f, 0.25f });
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> SphereMaterialAsset(TEXT("/Game/Materials/MATINST_AttachPointNode.MATINST_AttachPointNode"));
	if (SphereMaterialAsset.Succeeded())
	{
		AttachPointSphere->SetMaterial(0, SphereMaterialAsset.Object);
	}
}

void UShipAttachPoint::PostInitProperties()
{
	Super::PostInitProperties();

	// Use owner defaults if we have no compatible parts set.
	OwningShipPart = Cast<AShipPart>(GetOwner());
	if (OwningShipPart && CompatibleParts.Num() == 0)
	{
		CompatibleParts = OwningShipPart->GetDefaultCompatibleParts();
	}
}

void UShipAttachPoint::BeginPlay()
{
	Super::BeginPlay();	

	if (!SphereDMI)
	{
		SphereDMI = AttachPointSphere->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void UShipAttachPoint::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

bool UShipAttachPoint::IsCompatibleWith(const UShipAttachPoint* OtherPoint) const
{
	if (IsAttached() || !OtherPoint || OtherPoint->IsAttached())
		return false;
	const auto& OtherParts = OtherPoint->GetCompatibleParts();
	return CompatibleParts.ContainsByPredicate([&OtherParts](const auto& Part) { return OtherParts.Contains(Part); });
}

//void UShipAttachPoint::InitializeComponent()
//{
//	throw std::logic_error("The method or operation is not implemented.");
//}

bool UShipAttachPoint::IsAttached() const
{
	return (AttachedToShipPart != nullptr);
}

void UShipAttachPoint::AttachToShipPart(AShipPart* PartToAttachTo)
{
	AttachedToShipPart = PartToAttachTo;
}

void UShipAttachPoint::SetHighlighted(bool bHighlighted)
{
	if (bHighlighted == bIsHighlighted || !ensureMsgf(SphereDMI != nullptr, L"SphereDMI not set"))
		return;

	const FLinearColor NewColor = bHighlighted ? FLinearColor{ 0.f, 1.f, 0.f, 1.f } : FLinearColor{ 1.f, 1.f, 1.f, 1.f };
	SphereDMI->SetVectorParameterValue(TEXT("Color"), NewColor);

	bIsHighlighted = bHighlighted;
}
