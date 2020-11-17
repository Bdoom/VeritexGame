// Daniel Gleason (C) 2017

#include "VeritexTemperatureVolume.h"

#include "Veritex.h"
#include "TimerManager.h"
#include "VeritexCharacter.h"
#include "VeritexWorldSettings.h"

AVeritexTemperatureVolume::AVeritexTemperatureVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Col"));
	RootComponent = SphereCollision;
	SphereCollision->InitSphereRadius(500.f);
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAll"));
	SphereCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(PlayerCollisionChannel, ECR_Ignore);

	bReplicates = true;
}

void AVeritexTemperatureVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{	
		FTimerHandle TemperatureHandle;

		Temperature = FMath::RandRange(TemperatureMinimum, TemperatureMax);

		GetWorld()->GetTimerManager().SetTimer(TemperatureHandle, this, &AVeritexTemperatureVolume::ChooseNewTemperature, 300.f, true);
		// Make it check night or day for setting temperature
	}
}


void AVeritexTemperatureVolume::ChooseNewTemperature()
{
	float NewTemperature = FMath::RandRange(TemperatureMinimum, TemperatureMax);

	AVeritexWorldSettings* WorldSettings = GetWorldSettings();
	
	Temperature = NewTemperature;

	if (WorldSettings)
	{
		bool bNightTime = WorldSettings->IsNightTime();
		bool bMidDay = WorldSettings->IsMidDay();

		if (bNightTime)
		{
			Temperature -= Temperature * TemperatureModifier;
		}

		if (bMidDay)
		{
			Temperature += Temperature * TemperatureModifier;
		}
	}

}

AVeritexWorldSettings* AVeritexTemperatureVolume::GetWorldSettings()
{
	return Cast<AVeritexWorldSettings>(GetWorld()->GetWorldSettings());
}

void AVeritexTemperatureVolume::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVeritexTemperatureVolume, Temperature);
}
