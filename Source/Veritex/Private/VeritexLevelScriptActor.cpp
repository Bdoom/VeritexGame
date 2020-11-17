// Daniel Gleason (C) 2017

#include "VeritexLevelScriptActor.h"
#include "VeritexWorldSettings.h"

AVeritexLevelScriptActor::AVeritexLevelScriptActor()
{
}

AVeritexWorldSettings* AVeritexLevelScriptActor::GetWorldSettings()
{
	return Cast<AVeritexWorldSettings>(GetWorld()->GetWorldSettings());
}
