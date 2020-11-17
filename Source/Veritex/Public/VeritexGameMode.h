// Daniel Gleason (C) 2017

#pragma once
#include "VeritexGameModeBase.h"
#include "Structure.h"
#include "VeritexMasterInventoryComponent.h"
#include "VeritexFunctionLibrary.h"
#include "Item.h"

#include "VeritexGameMode.generated.h"

class AVeritexAIController;

UCLASS(minimalapi)
	class AVeritexGameMode : public AVeritexGameModeBase
{
	GENERATED_BODY()

public:

	AVeritexGameMode();

	// 	UPROPERTY(EditAnywhere, Category = "Loading")
	// 		TSubclassOf<AActor> LoadingBlockerBP_Class;

	UPROPERTY()
	TArray<FVeritexFoliageDataSet> FoliageData;

	UPROPERTY(EditAnywhere, Category = "Stasis System")
	float StasisDistance = 30000.f;

	DECLARE_DELEGATE_OneParam(FWorldDataLoaded, FWorldData const&);
	static FWorldDataLoaded WorldDataLoaded;
	void UniverseLoaded(FWorldData const& WorldData);

	//DECLARE_DELEGATE_ThreeParams(FCharacterDataLoaded, FCharacterData const&, bool, const FVector);
	//static FCharacterDataLoaded OnCharacterDataLoaded_Delegate;
	//void OnCharacterDataLoaded(FCharacterData const& Character, bool bKeepAlive, const FVector SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = Loading)
	void LoadUniverse();

	UPROPERTY()
	TArray<AStructure*> Structures;

	void Tick(float DeltaSeconds) override;

	UPROPERTY()
	TArray<FVector> PlayerLocations;

	UPROPERTY()
	int ActiveAINum;

	UPROPERTY()
	int StasisAINum;

	UPROPERTY()
	TArray<AVeritexAIController*> AIControllers;

	UFUNCTION()
	bool IsLocationNearbyPlayer(const FVector& Location, float Radius) const;

	UFUNCTION(BlueprintPure, Category = WorldSettings)
	AVeritexWorldSettings* GetWorldSettings();

	UPROPERTY(BlueprintReadWrite, Category = SpoilageTimers)
	TMap<FString, FTimerHandle> SpoilageTimers;

	UFUNCTION(BlueprintCallable, Category = SpoilageTimers)
	void StartSpoilageTimer(const FString& ItemID, float SpoilageTimer, UActorComponent* StorageComponent);

	UFUNCTION(BlueprintImplementableEvent, Category = SpoilageTimers)
	void SpoilItem(const FString& ItemID, UActorComponent* StorageComponent);

	UFUNCTION(BlueprintPure)
	bool PlayingInEditor();

	// FString = UniqueNetID, Character = that players character, used for finding player by UniqueSteamID
	UPROPERTY(BlueprintReadWrite, Category = Players)
	TMap<FString, AVeritexCharacter*> Players;

	UPROPERTY()
	TArray<AActor*> SpawnLocations;

	UFUNCTION(BlueprintPure, Category = Players)
	AVeritexCharacter* GetPlayerByUniqueSteamID(FString UniqueSteamID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Server)
	bool IsDedicatedServer();

	UPROPERTY(BlueprintReadOnly, Category = Database)
	FString DB_Name = FString("Veritex");

	UPROPERTY(BlueprintReadOnly, Category = Database)
	FString DB_Player_Table_Name = FString("Players");

	UPROPERTY(BlueprintReadOnly, Category = Database)
	FString DB_Structures_Table_Name = FString("Structures");

	UPROPERTY(BlueprintReadOnly, Category = Database)
	FString DB_Player_Inventory_Table_Prefix = FString("Inventory_");

	void PostLoad() override;

	void PostLogin(APlayerController* NewPlayer) override;

public:
	void SaveWorld();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AItem> ItemClass;

private:

	FTimerHandle SaveWorldTimerHandle;

	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	//virtual void PostLogin(APlayerController* NewPlayer) override;

	void Logout(AController* ExitingPlayer) override;

	AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	bool ShouldSpawnAtStartSpot(AController* Player) override;
};
