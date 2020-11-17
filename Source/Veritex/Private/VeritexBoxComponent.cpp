// Daniel Gleason (C) 2017

#include "VeritexBoxComponent.h"
#include "Veritex.h"

UVeritexBoxComponent::UVeritexBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
}
