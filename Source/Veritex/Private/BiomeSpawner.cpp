// Daniel Gleason (C) 2017

#include "BiomeSpawner.h"
#include "Veritex.h"
#include "TimerManager.h"
#include "InstancedFoliageActor.h"
#include "NavigationSystem.h"
#include "Engine/LevelStreaming.h"
#include "Components/SceneComponent.h"
#include "VeritexGameMode.h"
#include "Components/BrushComponent.h"
#include "GameFramework/GameStateBase.h"

// Some reason I think other tiles are loading shit too... also loading on clients??

ABiomeSpawner::ABiomeSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	MySceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));
	RootComponent = MySceneComponent;
}

void ABiomeSpawner::LevelLoaded()
{
	if (HasAuthority())
	{
		NumberOfLevelsRequiredToBeLoadedBeforeSpawning++;

		if (NumberOfLevelsRequiredToBeLoadedBeforeSpawning >= 25) //30
		{
			SpawnNPCs();
		}
	}
}

void ABiomeSpawner::SpawnNPCs()
{
	if (HasAuthority())
	{
		GSB = GetWorld()->GetAuthGameMode()->GameState;

		for (; RandomLocationsToSpawnNPCs.Num() < NumberOfNPCsToSpawnInThisBiome;)
		{
			RandomLocationsToSpawnNPCs.AddUnique(RandomPointWithinVolumeGrounded());
		}
	}
}

void ABiomeSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnerTick, this, &ABiomeSpawner::TickNPCs, 10.f, true, 5.f);
	}
}

void ABiomeSpawner::TickNPCs()
{
	SCOPE_CYCLE_COUNTER(STAT_TickNPCs_Start);
}


FVector ABiomeSpawner::RandomPointWithinVolume()
{
	//GetBrushComponent()->Brush->Bounds
	if (GetBrushComponent())
	{
		FVector BoxExtent = FVector(GetBrushComponent()->Brush->Bounds.BoxExtent.X,
		                            GetBrushComponent()->Brush->Bounds.BoxExtent.Y,
		                            GetBrushComponent()->Brush->Bounds.BoxExtent.Z);
		const FVector BoxMin = GetActorLocation() - BoxExtent;
		const FVector BoxMax = GetActorLocation() + BoxExtent;
		return FMath::RandPointInBox(FBox(BoxMin, BoxMax));
	}
	return FVector::ZeroVector;
}

FVector ABiomeSpawner::RandomPointWithinVolumeGrounded()
{
	FVector RandomPoint = RandomPointWithinVolume();

	FHitResult OutHit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GetWorld()));

	bool bSuccessful = GetWorld()->LineTraceSingleByChannel(OutHit, RandomPoint,
	                                                        RandomPoint + (GetActorUpVector() * -5000000),
	                                                        ECC_WorldStatic, Params);

	if (bSuccessful)
	{
		FVector LocationToSpawn = OutHit.Location + (GetActorUpVector() * 150);

		return LocationToSpawn;
	}

	return FVector::ZeroVector;
}

int32 ABiomeSpawner::GetOmnivoresInThisBiome()
{
	return OmnivoresInThisBiome;
}

int32 ABiomeSpawner::GetNPCsInThisBiome()
{
	return NPCsInThisBiome;
}

int32 ABiomeSpawner::GetCarnivoresInThisBiome()
{
	return CarnivoresInThisBiome;
}

int32 ABiomeSpawner::GetHerbivoresInThisBiome()
{
	return HerbivoresInThisBiome;
}
