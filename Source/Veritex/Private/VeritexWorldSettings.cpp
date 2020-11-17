// Daniel Gleason (C) 2017

#include "VeritexWorldSettings.h"

#include "Veritex.h"
#include "VeritexLevelScriptActor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LevelStreaming.h"

AVeritexWorldSettings::AVeritexWorldSettings()
{
	//bUseClientSideLevelStreamingVolumes = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.bCanEverTick = true;

	if (IsNetMode(NM_ListenServer))
	{
		bEnableWorldComposition = false;
	}

	// 	AVeritexLevelScriptActor* LevelScriptActor = Cast<AVeritexLevelScriptActor>(GetWorld()->GetLevelScriptActor());
	// 	if (LevelScriptActor)
	// 	{
	// 		if (LevelScriptActor->SkyDome)
	// 		{
	// 			SkyDome = LevelScriptActor->SkyDome;
	// 		}
	// 	}
}

bool AVeritexWorldSettings::IsNightTime()
{
	if (MoonLightIntensity > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AVeritexWorldSettings::IsMidDay()
{
	if (TimeOfDay == 1200 || (TimeOfDay < 1700 && TimeOfDay > 1100))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void AVeritexWorldSettings::NotifyBeginPlay()
{
	// This is a hack because Listen servers aren't loading collision and tiles for players 
	if (IsNetMode(NM_ListenServer) || IsNetMode(NM_Client))
		//{
		bEnableWorldComposition = false;

	if (GetWorld()->WorldComposition)
	{
		for (class ULevelStreaming* Level : GetWorld()->WorldComposition->TilesStreaming)
		{
			Level->bDisableDistanceStreaming = true;
		}
	}

	if (GetLevel()->IsPersistentLevel())
	{
		for (class ULevelStreaming* Level : GetWorld()->GetStreamingLevels())
		{
			Level->SetShouldBeLoaded(true);
			Level->SetShouldBeVisible(true);
			Level->bShouldBlockOnLoad = true;
		}
	}
	GetWorld()->FlushLevelStreaming(EFlushLevelStreamingType::Full);


	Super::NotifyBeginPlay();
}

bool AVeritexWorldSettings::IsDayTime()
{
	if (SunLightIntensity > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void AVeritexWorldSettings::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVeritexWorldSettings, TimeOfDay);
	DOREPLIFETIME(AVeritexWorldSettings, MoonLightIntensity);
	DOREPLIFETIME(AVeritexWorldSettings, SunLightIntensity);
}
