// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/PlayerController.h"
#include "Structure.h"
#include "VeritexCharacter.h"
#include "GameFramework/PlayerInput.h"
#include "VeritexPlayerController.generated.h"

/**
 *
 */
UCLASS()
class VERITEX_API AVeritexPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AVeritexPlayerController();

	UFUNCTION(BlueprintPure, Category = "Keybinds")
		TArray<struct FInputActionKeyMapping> GetActionMappings();

	UFUNCTION(BlueprintPure, Category = "Keybinds")
		TArray<struct FInputAxisKeyMapping> GetAxisMappings();

	UFUNCTION(BlueprintPure, Category = "Keybinds")
		class UPlayerInput* GetPlayerInput();

	UFUNCTION(BlueprintCallable, Category = "Keybinds")
		void SetNewActionKeyMapping(FName ActionName, FInputActionKeyMapping NewActionMapping);

	UFUNCTION(Client, Reliable)
		void Client_LoadWorldTiles(FVector SpawnPoint);

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> LoadingBlockerBP_Class;

	UFUNCTION(BlueprintImplementableEvent)
		void SetMovementSpeedFromSave();

	UPROPERTY(Replicated)
		bool KeepAlive;

	UPROPERTY()
		class UVeritexMasterInventoryComponent* Inventory;

	UPROPERTY(Replicated)
		FCharacterData CharacterData;

	UFUNCTION(BlueprintCallable)
		void OnInventoryReady();

	void CouldNotFindSaveData(FVector& SpawnPoint, const FString& PassedUniqueSteamID);
	void OnCharacterDataLoaded(FCharacterData const& Character, bool bKeepAlive, FVector SpawnPoint);

	UFUNCTION(exec)
		void SetUniqueSteamID(FString NewSteamID);

	UFUNCTION(Server, WithValidation, Unreliable)
		void Server_SetUniqueSteamID(const FString& NewSteamID);

	UPROPERTY()
		bool bHasSeenWorldMap;

	UFUNCTION(BlueprintPure)
		bool IsListenServer();

	UFUNCTION(BlueprintPure)
		bool IsClient();

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void Server_TryLoadUser(const FString& PassedUniqueSteamID, bool bKeepAlive, FVector SpawnPoint);

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void Client_TryLoadUser();

	UFUNCTION(BlueprintPure, Category = "Replication")
		ENetRole GetRole() const;

	UPROPERTY()
		bool IsFirstPossess = true;

	UFUNCTION(BlueprintCallable, Category = Steam)
		void Client_Travel_To_Server(FString ServerIP);

	UFUNCTION(BlueprintPure)
		bool PlayingInEditor();

	UFUNCTION(BlueprintPure, Category = Foliage)
		AInstancedFoliageActor* GetInstancedFoliageActor();

	UFUNCTION(BlueprintPure, Category = Steam)
		FString GetSteamID();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Character)
		AVeritexCharacter* MyCharacter;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Player Data")
		void SavePlayer(bool bIncludeInventory);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnUnPossess() override;

	virtual void OnPossess(APawn* aPawn) override;

	virtual void PawnLeavingGame() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
		bool isControllerPossessingPawn = false;

	UPROPERTY(BlueprintReadOnly, Category = Pawn)
		APawn* LastPossessedPawn;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = Structures)
		bool PlacingStructure;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		bool PlaceTheStructure;

	UPROPERTY()
		FString HeroClassPath;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UFUNCTION(Server, WithValidation, Reliable)
		void Server_KillPlayer();

	UFUNCTION(exec)
		void Suicide();

	UPROPERTY(BlueprintReadWrite, Replicated)
		FString UniqueSteamID;

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = Actor)
		void Server_DestroyActor(AActor* TheActorToDestroy);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "World Map")
		TSoftClassPtr<class UUserWidget> WorldMap_WidgetClass;

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void Client_OpenWorldMapWidget();


	FTimerHandle SavePlayerHandle;
	FTimerDelegate SavePlayerDelegate;


	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;

protected:
	virtual void SetupInputComponent() override;


};
