// Daniel Gleason (C) 2017

#include "VeritexCharacterMovement.h"
#include "Veritex.h"

UVeritexCharacterMovement::UVeritexCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

 
float UVeritexCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();
	
	return MaxSpeed;
}
