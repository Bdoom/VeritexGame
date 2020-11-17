// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "GameFramework/Character.h"
#include "NPC.h"
#include "BiomeSpawner.generated.h"

DECLARE_STATS_GROUP(TEXT("TickNPCs"), STATGROUP_TickNPCs, STATCAT_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("TickNPCs Start"), STAT_TickNPCs_Start, STATGROUP_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("Looping Through PlayerStates"), STAT_TickNPCs_PlayerStates_Looping, STATGROUP_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("Looping Through SpawnPoints"), STAT_TickNPCs_SpawnPoints_Looping, STATGROUP_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("DistSquared Test"), STAT_TickNPCs_DistSquared_Test, STATGROUP_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("Actual Spawning of NPC"), STAT_TickNPCs_Spawning_NPC, STATGROUP_TickNPCs);
DECLARE_CYCLE_STAT(TEXT("Setting NPC Stats"), STAT_TickNPCs_Setting_Stats, STATGROUP_TickNPCs);


UCLASS()
class VERITEX_API ABiomeSpawner : public AVolume
{
	GENERATED_BODY()

public:

	ABiomeSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
		TSubclassOf<ANPC> NpcClass;

	UPROPERTY(VisibleAnywhere)
		class USceneComponent* MySceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 NPC_MinLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 NPC_MaxLevel;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		TArray<TSubclassOf<ANPC>>  AI_To_Spawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		TArray<ANPC*>  AI_In_This_Biome; // Remember it has a max size per biome.

	// Returns a random point in the volume that is line traced to the ground.
	UFUNCTION(BlueprintPure)
		FVector RandomPointWithinVolume();

	UFUNCTION()
		FVector RandomPointWithinVolumeGrounded();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 NPCsInThisBiome;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 CarnivoresInThisBiome;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 HerbivoresInThisBiome;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 OmnivoresInThisBiome;

	UFUNCTION(BlueprintPure, Category = NPCs)
		int32 GetOmnivoresInThisBiome();

	UFUNCTION(BlueprintPure, Category = NPCs)
		int32 GetNPCsInThisBiome();

	UFUNCTION(BlueprintPure, Category = NPCs)
		int32 GetCarnivoresInThisBiome();

	UFUNCTION(BlueprintPure, Category = NPCs)
		int32 GetHerbivoresInThisBiome();

	UFUNCTION()
		void TickNPCs();

	UPROPERTY(EditAnywhere, Category = NPCs)
		int32 NumberOfNPCsToSpawnPerTick = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 NumberOfNPCsToSpawnInThisBiome = 300;

	UPROPERTY()
		FTimerHandle SpawnerTick;

	UPROPERTY()
		AGameStateBase* GSB;

	UPROPERTY()
		int16 NumberOfLevelsRequiredToBeLoadedBeforeSpawning;

	UFUNCTION()
		void SpawnNPCs();

	UFUNCTION()
		void LevelLoaded();

	UPROPERTY()
		TArray<FVector> RandomLocationsToSpawnNPCs;

};
