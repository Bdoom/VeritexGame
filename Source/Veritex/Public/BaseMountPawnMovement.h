// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "BaseMountPawnMovement.generated.h"

UCLASS()
class VERITEX_API UBaseMountPawnMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual float GetMaxSpeed() const override;

	FTraceDelegate CapsuleSweepDelegate;
	FHitResult CapsuleSweepHit;

	UBaseMountPawnMovement(const FObjectInitializer& ObjectInitializer);

	void FinishCapsuleSweep(FTraceHandle const& Handle, FTraceDatum& Datum);

	void StartCapsuleSweep();
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void InitializeComponent() override;


	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult = NULL) const override;

protected:
	virtual void OnCharacterStuckInGeometry(const FHitResult* Hit) override;

	virtual bool ShouldComputePerchResult(const FHitResult& InHit, bool bCheckRadius = true) const override;

};
