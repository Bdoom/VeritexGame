// Daniel Gleason (C) 2017


#include "VeritexPlayerState.h"
#include "Veritex.h"

AVeritexPlayerState::AVeritexPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetShouldUpdateReplicatedPing(false);
}

void AVeritexPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


