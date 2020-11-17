// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "VeritexCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class VERITEX_API UVeritexCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UVeritexCharacterMovement(const FObjectInitializer& ObjectInitializer);

public:
	virtual float GetMaxSpeed() const override;

};
