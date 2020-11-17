// Daniel Gleason (C) 2017

#include "BaseMountPawnMovement.h"
#include "Veritex.h"
#include "NPC.h"

float UBaseMountPawnMovement::GetMaxSpeed() const
{
	return Super::GetMaxSpeed();

	// 	ANPC* Mount = Cast<ANPC>(GetOwner());
	// 	switch (Mount->MountName)
	// 	{
	// 	case EMountName::Phoenix:
	// 		return 10000;
	// 
	// 		
	// 	case EMountName::Dragon:
	// 		if (IsFlying())
	// 		{
	// 			return 300000;
	// 		}
	// 		else
	// 		{
	// 			return 1000;
	// 		}
	// 		
	// 	default:
	// 		return Super::GetMaxSpeed();
	// 		
	// 	}
}

UBaseMountPawnMovement::UBaseMountPawnMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;

	//bProjectNavMeshWalking = true;
	//bProjectNavMeshOnBothWorldChannels = true;
	//NavMeshProjectionHeightScaleUp = 1.f;
}

void UBaseMountPawnMovement::FinishCapsuleSweep(FTraceHandle const& Handle, FTraceDatum& Datum)
{
	if (Datum.OutHits.Num() > 0)
	{
		CapsuleSweepHit = Datum.OutHits.Last();
	}
	else
	{
		CapsuleSweepHit.Reset(-1.0f, false);
	}
}

void UBaseMountPawnMovement::StartCapsuleSweep()
{
	if (!IsPendingKill() && UpdatedComponent)
	{
		check(CharacterOwner);
		check(CharacterOwner->GetCapsuleComponent());
		UCapsuleComponent* const Capsule = CharacterOwner->GetCapsuleComponent();

		FVector const TraceStart = UpdatedComponent->GetComponentLocation();
		FVector const TraceEnd = TraceStart - FVector::UpVector * 10.0f;
		FCollisionShape const Shape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),
		                                                           Capsule->GetScaledCapsuleHalfHeight());

		FCollisionQueryParams QueryParams{SCENE_QUERY_STAT(MinionAsyncCapsuleSweep), false, CharacterOwner};
		FCollisionResponseParams ResponseParams;
		InitCollisionParams(QueryParams, ResponseParams);

		GetWorld()->AsyncSweepByChannel(
			EAsyncTraceType::Single, TraceStart, TraceEnd, ECC_Visibility, Shape, QueryParams, ResponseParams,
			&CapsuleSweepDelegate);
	}
	else
	{
		CapsuleSweepHit.Reset(-1.0f, false);
	}
}

void UBaseMountPawnMovement::BeginPlay()
{
	Super::BeginPlay();

	ANPC* Mount = Cast<ANPC>(GetOwner());

	CapsuleSweepDelegate = FTraceDelegate::CreateUObject(this, &UBaseMountPawnMovement::FinishCapsuleSweep);

	switch (Mount->MountName)
	{
	case EMountName::Dragon:
		AirControl = 1;
		break;

	default:

		break;
	}
}

void UBaseMountPawnMovement::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	StartCapsuleSweep();
}

void UBaseMountPawnMovement::InitializeComponent()
{
	Super::InitializeComponent();
}

void UBaseMountPawnMovement::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance,
                                              FFindFloorResult& OutFloorResult, float SweepRadius,
                                              const FHitResult* DownwardSweepResult /*= NULL*/) const
{
	if (UpdatedComponent && UpdatedComponent->GetComponentLocation().Equals(CapsuleLocation, 0.001f))
	{
		return Super::ComputeFloorDist(CapsuleLocation, LineDistance, SweepDistance, OutFloorResult, SweepRadius,
		                               &CapsuleSweepHit);
	}

	return Super::ComputeFloorDist(CapsuleLocation, LineDistance, SweepDistance, OutFloorResult, SweepRadius,
	                               DownwardSweepResult);
}

void UBaseMountPawnMovement::OnCharacterStuckInGeometry(const FHitResult* Hit)
{
	//Super::OnCharacterStuckInGeometry(Hit);
	if (!GetOwner()->IsPendingKill())
	{
		GetOwner()->Destroy();
	}
}

bool UBaseMountPawnMovement::ShouldComputePerchResult(const FHitResult& InHit, bool bCheckRadius) const
{
	return false;
}
