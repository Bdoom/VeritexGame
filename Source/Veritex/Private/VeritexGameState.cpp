// Daniel Gleason (C) 2017

#include "VeritexGameState.h"

#include "Veritex.h"
#include "VeritexWorldSettings.h"
#include "VeritexStructs.h"

void AVeritexGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVeritexGameState, WeatherMode);
}

AVeritexWorldSettings* AVeritexGameState::GetWorldSettings()
{
	return Cast<AVeritexWorldSettings>(GetWorld()->GetWorldSettings());
}

void AVeritexGameState::ChooseWeather()
{
	int32 RandomWeather = FMath::RandRange(0, 2);
	if (RandomWeather == 0)
	{
		WeatherMode = EWeatherMode::ClearSkys;
	}
	else if (RandomWeather == 1)
	{
		WeatherMode = EWeatherMode::Rain;
	}
	else if (RandomWeather == 2)
	{
		WeatherMode = EWeatherMode::Snow;
	}
}

void AVeritexGameState::BeginPlay()
{
	Super::BeginPlay();
	WeatherMode = EWeatherMode::Snow;
	if (HasAuthority())
	{
		FTimerHandle WeatherHandle;
		GetWorld()->GetTimerManager().SetTimer(WeatherHandle, this, &AVeritexGameState::ChooseWeather, 1800.f, true);
	}
}
