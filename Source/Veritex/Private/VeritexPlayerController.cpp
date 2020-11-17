// Daniel Gleason (C) 2017

#include "VeritexPlayerController.h"

#include "Veritex.h"
#include "VeritexCharacter.h"
#include "VeritexFunctionLibrary.h"
#include "VeritexGameMode.h"
#include "VeritexCheatManager.h"

#include "UWorksLibraryGameID.h"

#include "Sound/SoundWaveProcedural.h"
#include "Sound/AudioSettings.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"

#include "VeritexGameInstance.h"

#include "EngineUtils.h"

#include "VeritexPlayerState.h"

#include "BaseMountPawn.h"

#include "Engine/LevelStreaming.h"
#include "Engine/WorldComposition.h"

#include "Kismet/GameplayStatics.h"

#include "UWorksLibraryCore.h"
#include "Interfaces/UWorksInterfaceCoreUser.h"
#include "Libraries/UWorksLibrarySteamID.h"
#include "VeritexAsyncClasses.h"

#include "Runtime/UMG/Public/UMG.h"

#include "GameFramework/GameState.h"
#include "GameFramework/GameModeBase.h"

AVeritexPlayerController::AVeritexPlayerController()
{

}


TArray<struct FInputActionKeyMapping> AVeritexPlayerController::GetActionMappings()
{
	return PlayerInput->ActionMappings;
}

TArray<struct FInputAxisKeyMapping> AVeritexPlayerController::GetAxisMappings()
{
	return PlayerInput->AxisMappings;
}

class UPlayerInput* AVeritexPlayerController::GetPlayerInput()
{
	return PlayerInput;
}

void AVeritexPlayerController::SetNewActionKeyMapping(FName ActionName, FInputActionKeyMapping NewActionMapping)
{

}

void AVeritexPlayerController::Client_LoadWorldTiles_Implementation(FVector SpawnPoint)
{
	if (GetWorld()->WorldComposition)
	{
		GetWorld()->WorldComposition->UpdateStreamingState(SpawnPoint);
		TArray<FDistanceVisibleLevel> OutVisibleLevels;
		TArray<FDistanceVisibleLevel> OutHiddenLevels;
		GetWorld()->WorldComposition->GetDistanceVisibleLevels(SpawnPoint, OutVisibleLevels, OutHiddenLevels);

		for (FDistanceVisibleLevel LevelTile : OutVisibleLevels)
		{
			LevelTile.StreamingLevel->bDisableDistanceStreaming = true;
			LevelTile.StreamingLevel->SetShouldBeLoaded(true);
			LevelTile.StreamingLevel->SetShouldBeVisible(true);
		}
	}
}

void AVeritexPlayerController::SetUniqueSteamID(FString NewSteamID)
{
	Server_SetUniqueSteamID(NewSteamID);
}

void AVeritexPlayerController::Server_SetUniqueSteamID_Implementation(const FString& NewSteamID)
{
	UniqueSteamID = NewSteamID;
	MyCharacter->UniqueSteamID = NewSteamID;
}

bool AVeritexPlayerController::Server_SetUniqueSteamID_Validate(const FString& NewUniqueSteamID)
{
	return true;
}

bool AVeritexPlayerController::IsListenServer()
{
	return IsNetMode(ENetMode::NM_ListenServer);
}

bool AVeritexPlayerController::IsClient()
{
	return IsNetMode(ENetMode::NM_Client);
}

ENetRole AVeritexPlayerController::GetRole() const
{
	return Role;
}


void AVeritexPlayerController::Client_Travel_To_Server(FString ServerIP)
{
	ClientTravel(ServerIP, ETravelType::TRAVEL_Absolute);
}

bool AVeritexPlayerController::PlayingInEditor()
{
	return GetWorld()->IsPlayInEditor();
}

AInstancedFoliageActor* AVeritexPlayerController::GetInstancedFoliageActor()
{
	return AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GetWorld());
}

FString AVeritexPlayerController::GetSteamID()
{
	//check(Role != ROLE_Authority && "GetSteamID() Cannot be run on server");
	FString steamid = "0";
	if (UUWorksLibraryCore::GetUser())
	{
		steamid = UUWorksLibrarySteamID::GetIdentifier(UUWorksLibraryCore::GetUser()->GetSteamID());
	}
	return steamid;
}


void AVeritexPlayerController::OnCharacterDataLoaded(FCharacterData const& character, bool bKeepAlive, FVector SpawnPoint)
{
	if (!HasAuthority())
	{
		return;
	}

	FString ClassPath = "/Game/Blueprints/VeritexCharacter_BP.VeritexCharacter_BP_C";
	UClass* CharacterClass = LoadObject<UClass>(nullptr, (*ClassPath), nullptr, 0, nullptr);
	CharacterData = character;
	KeepAlive = bKeepAlive;

	FTransform SpawnTransform;
	if (!SpawnPoint.IsNearlyZero())
	{
		SpawnTransform.SetLocation(SpawnPoint);
	}
	else
	{
		SpawnTransform = character.PlayerPosition;
	}

	Client_LoadWorldTiles(SpawnTransform.GetLocation());

	if (!IsNetMode(NM_DedicatedServer)) // Added to stop player from falling thru world. 
	{
		GetWorld()->SpawnActor<AActor>(LoadingBlockerBP_Class, SpawnTransform);
	}

	AVeritexCharacter* VChar = GetWorld()->SpawnActor<AVeritexCharacter>(CharacterClass, SpawnTransform);

	Inventory = VChar->FindComponentByClass<UVeritexMasterInventoryComponent>();

	Possess(VChar);
}

void AVeritexPlayerController::OnInventoryReady()
{
	// This does not need to be called on new characters / players... This is only for loading data from save.

	if (!HasAuthority())
	{
		return;
	}

	if (KeepAlive)
	{
		MyCharacter->Health = CharacterData.MaxHealth > 0 ? CharacterData.MaxHealth : 100.f;
	}
	else
	{
		MyCharacter->Health = CharacterData.Health > 0 ? CharacterData.Health : 100.f;
	}

	MyCharacter->MaxHealth = CharacterData.MaxHealth > 0 ? CharacterData.MaxHealth : 100;
	if (KeepAlive)
	{
		MyCharacter->Food = CharacterData.MaxFood > 0 ? CharacterData.MaxFood : 100;
	}
	else
	{
		MyCharacter->Food = CharacterData.Food;
	}
	MyCharacter->CurrentLevel = CharacterData.Level > 0 ? CharacterData.Level : 1;
	MyCharacter->Experience = CharacterData.Experience;
	if (Inventory)
	{
		bool bHasHatchet = false;
		for (FItemData Item : CharacterData.Inventory)
		{
			Inventory->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
			if (Item.ItemID.Equals("Hatchet"))
			{
				bHasHatchet = true;
			}
		}
		for (FItemData EquipmentItem : CharacterData.Equipment)
		{
			Inventory->EquipPlayerItem(EquipmentItem.ItemID, EquipmentItem.Count);
		}
		Inventory->MaxWeight = CharacterData.MaxWeight;

		if (!bHasHatchet)
		{
			Inventory->AddPlayerItem("Hatchet", 1, 0);
		}

		MyCharacter->MaxStamina = CharacterData.MaxStamina;
	}
	MyCharacter->LevelUpPoints = CharacterData.LevelUpPoints;
	MyCharacter->MaxFood = CharacterData.MaxFood > 0 ? CharacterData.MaxFood : 100;
	MyCharacter->MaxOxygen = CharacterData.MaxOxygen > 0 ? CharacterData.MaxOxygen : 100;
	MyCharacter->MaxWater = CharacterData.MaxWater > 0 ? CharacterData.MaxWater : 100;
	MyCharacter->MeleeDamage = CharacterData.MeleeDamage > 20 ? CharacterData.MeleeDamage : 20;
	MyCharacter->MovementSpeed = CharacterData.MovementSpeed;

	MyCharacter->UniqueSteamID = CharacterData.UniqueSteamID;

	if (KeepAlive)
	{
		MyCharacter->Water = CharacterData.MaxWater > 0 ? CharacterData.MaxWater : 100;
	}
	else
	{
		MyCharacter->Water = CharacterData.Water;
	}

	SetMovementSpeedFromSave();

	if (MyCharacter->Health <= 0)
	{
		Client_OpenWorldMapWidget();
		if (!MyCharacter->IsPendingKill())
		{
			MyCharacter->Destroy();
		}
		return;
	}

}

void AVeritexPlayerController::Server_TryLoadUser_Implementation(const FString& PassedUniqueSteamID, bool bKeepAlive, FVector SpawnPoint)
{
	UniqueSteamID = PassedUniqueSteamID; // Passed from client to server.

	if (GetWorld()->GetMapName().Contains("Voxel"))
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(UVeritexFunctionLibrary::PathToCharacters() + PassedUniqueSteamID + FString("_Creative") + FString(".json"))))
		{
			FCharacterData CharData;
			FString JsonString;
			if (FFileHelper::LoadFileToString(JsonString, *(UVeritexFunctionLibrary::PathToCharacters() + UniqueSteamID + FString("_Creative") + FString(".json"))))
			{
				JsonString = UVeritexFunctionLibrary::Decrypt(JsonString);
				FJsonObjectConverter::JsonObjectStringToUStruct<FCharacterData>(JsonString, &CharData, 0, 0);
				OnCharacterDataLoaded(CharData, bKeepAlive, SpawnPoint);
			}


			//(new FAutoDeleteAsyncTask<FSaveLoadWorldTask>(false, PassedUniqueSteamID, bKeepAlive, SpawnPoint))->StartBackgroundTask();
		}
		else
		{
			CouldNotFindSaveData(SpawnPoint, PassedUniqueSteamID);
		}

	}
	else
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(UVeritexFunctionLibrary::PathToCharacters() + PassedUniqueSteamID + FString(".json"))))
		{
			FCharacterData CharData;
			FString JsonString;
			if (FFileHelper::LoadFileToString(JsonString, *(UVeritexFunctionLibrary::PathToCharacters() + UniqueSteamID + FString(".json"))))
			{
				JsonString = UVeritexFunctionLibrary::Decrypt(JsonString);
				FJsonObjectConverter::JsonObjectStringToUStruct<FCharacterData>(JsonString, &CharData, 0, 0);
				OnCharacterDataLoaded(CharData, bKeepAlive, SpawnPoint);
			}


			//(new FAutoDeleteAsyncTask<FSaveLoadWorldTask>(false, PassedUniqueSteamID, bKeepAlive, SpawnPoint))->StartBackgroundTask();
		}
		else
		{
			CouldNotFindSaveData(SpawnPoint, PassedUniqueSteamID);
		}
	}

}

void AVeritexPlayerController::CouldNotFindSaveData(FVector& SpawnPoint, const FString& PassedUniqueSteamID)
{

	if (!bHasSeenWorldMap)
	{
		Client_OpenWorldMapWidget();
		bHasSeenWorldMap = true;
	}
	else
	{
		FString ClassPath = "/Game/Blueprints/VeritexCharacter_BP.VeritexCharacter_BP_C";
		UClass* CharacterClass = LoadObject<UClass>(nullptr, (*ClassPath), nullptr, 0, nullptr);
		AGameModeBase * GMB = GetWorld()->GetAuthGameMode();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnPoint);

		Client_LoadWorldTiles(SpawnTransform.GetLocation());

		if (!IsNetMode(NM_DedicatedServer))
		{
			GetWorld()->SpawnActor<AActor>(LoadingBlockerBP_Class, SpawnTransform);
		}


		AVeritexCharacter* Char = GetWorld()->SpawnActor<AVeritexCharacter>(CharacterClass, SpawnTransform);
		Char->UniqueSteamID = PassedUniqueSteamID;
		UVeritexMasterInventoryComponent* bag = Char->FindComponentByClass<UVeritexMasterInventoryComponent>();
		if (bag)
		{
			bag->SetShouldCallOnInventoryReady(false);
		}


		Possess(Char);
		SavePlayer(true);

	}

}

void AVeritexPlayerController::Client_OpenWorldMapWidget_Implementation()
{
	UClass* WorldMapClass = WorldMap_WidgetClass.Get();
	UUserWidget* WorldMap_Widget = CreateWidget<UUserWidget>(GetWorld(), WorldMapClass);
	WorldMap_Widget->AddToViewport();
	bShowMouseCursor = true;
}

bool AVeritexPlayerController::Server_TryLoadUser_Validate(const FString& PassedUniqueSteamID, bool bKeepAlive, FVector SpawnPoint)
{
	return true;
}

void AVeritexPlayerController::Client_TryLoadUser_Implementation()
{
	Server_TryLoadUser(GetSteamID(), false, FVector(0, 0, 0));
}

void AVeritexPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//PlayerCameraManager->bUseClientSideCameraUpdates = false; // Vorixo said to do this, if it fucks shit up blame him. 

	UniqueSteamID = GetSteamID();

	if (HasAuthority())
	{
		SavePlayerDelegate.BindUFunction(this, FName("SavePlayer"), true);

		GetWorld()->GetTimerManager().SetTimer(SavePlayerHandle, SavePlayerDelegate, 5.f, true);
	}
}

void AVeritexPlayerController::SavePlayer(bool bIncludeInventory)
{
	AVeritexCharacter* Char = Cast<AVeritexCharacter>(GetPawn());

	FCharacterData CharData;

	if (Char)
	{

		UVeritexMasterInventoryComponent* InventoryComponent = Char->FindComponentByClass<UVeritexMasterInventoryComponent>();

		if (bIncludeInventory)
		{
			if (InventoryComponent)
			{
				InventoryComponent->SetInventoryArrayForCPP();
				InventoryComponent->SetEquipmentArrayForCPP();

				TArray<FItemData> InventoryItems = InventoryComponent->GetInventoryItems();
				TArray<FItemData> Equipment = InventoryComponent->GetEquipmentItems();
				CharData.Equipment = Equipment;
				CharData.Inventory = InventoryItems;

			}
		}


		CharData.MaxStamina = Char->MaxStamina;

		CharData.Level = Char->CurrentLevel;
		CharData.Experience = Char->Experience;
		CharData.Food = Char->Food;
		CharData.Health = Char->Health;
		CharData.LevelUpPoints = Char->LevelUpPoints;
		CharData.MaxFood = Char->MaxFood;
		CharData.MaxHealth = Char->MaxHealth;
		CharData.MaxOxygen = Char->MaxOxygen;
		CharData.MaxWater = Char->MaxWater;
		if (InventoryComponent)
		{
			CharData.MaxWeight = InventoryComponent->MaxWeight;
		}
		CharData.MeleeDamage = Char->MeleeDamage;
		CharData.MovementSpeed = Char->MovementSpeed;
		CharData.PlayerPosition = Char->GetActorTransform();
		CharData.UID = Char->UID;
		if (!Char->UniqueSteamID.Equals(" "))
		{
			CharData.UniqueSteamID = UniqueSteamID;
		}
		else
		{
			CharData.UniqueSteamID = Char->UniqueSteamID;
		}
		CharData.Water = Char->Water;

		if (!bIncludeInventory) // If false, players should be set to "0" hp, thus dead so they can respawn.
		{
			CharData.Health = 0;
			CharData.Food = 0;
			CharData.Water = 0;
		}

		if (GetWorld()->GetMapName().Contains("Voxel"))
		{
			(new FAutoDeleteAsyncTask<FSaveLoadWorldTask>(true, CharData.UniqueSteamID, CharData, true))->StartBackgroundTask();
		}
		else
		{
			(new FAutoDeleteAsyncTask<FSaveLoadWorldTask>(true, CharData.UniqueSteamID, CharData, false))->StartBackgroundTask();
		}

	}
}

void AVeritexPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void AVeritexPlayerController::Server_DestroyActor_Implementation(AActor* TheActorToDestroy)
{
	if (TheActorToDestroy != nullptr)
	{
		if (!TheActorToDestroy->IsPendingKillPending())
		{
			TheActorToDestroy->Destroy();
		}
	}
}

bool AVeritexPlayerController::Server_DestroyActor_Validate(AActor* TheActorToDestroy)
{
	return true;
}

void AVeritexPlayerController::OnUnPossess()
{
	isControllerPossessingPawn = false;
	// Store Data For the PlayerPawn before OnLogout() is called in the game mode.

	if (GetPawn() != NULL)
	{
		LastPossessedPawn = GetPawn();
	}

	Super::OnUnPossess();

	if (LastPossessedPawn != nullptr) // For some reason Unpossess() is called when you close the player controller blueprint, i'm assuming the same happens with Possess() when you open it.
	{
		if (LastPossessedPawn->IsA<AVeritexCharacter>())
		{
			LastPossessedPawn->SetOwner(this);
			AVeritexCharacter* VCharacter = Cast<AVeritexCharacter>(LastPossessedPawn);
			VCharacter->SetActorTickEnabled(false);
		}
	}

}

void AVeritexPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ANPC* NPC = Cast<ANPC>(GetPawn());
	if (NPC && NPC->GetName().Contains("Raft") && MyCharacter && MyCharacter->Health <= 0)
	{
		UnPossess(); // If we die while on the raft, unpossess the raft. 
	}

}


void AVeritexPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVeritexPlayerController, PlacingStructure);
	DOREPLIFETIME(AVeritexPlayerController, MyCharacter);
	DOREPLIFETIME(AVeritexPlayerController, UniqueSteamID);
	DOREPLIFETIME(AVeritexPlayerController, KeepAlive);
	DOREPLIFETIME(AVeritexPlayerController, CharacterData);
}

void AVeritexPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

}

void AVeritexPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	AVeritexCharacter* Char = Cast<AVeritexCharacter>(aPawn);
	if (IsFirstPossess)
	{
		//Super::Possess(aPawn);
		IsFirstPossess = false;
		if (GetPawn() != nullptr)
		{
			HeroClassPath = GetPawn()->GetClass()->GetPathName();
		}
	}

	isControllerPossessingPawn = true;
	LastPossessedPawn = aPawn;
	// ON Possess of any pawn by this controller, attempt to cast it to veritexcharacter and set it to be logged in

	if (Char)
	{
		Char->IsLoggedIn = true; // IsLoggedIn is replicated, must be called on server

		MyCharacter = Char;
		AVeritexPlayerState* VPS = Cast<AVeritexPlayerState>(PlayerState);
		if (VPS != nullptr)
		{
			VPS->MyCharacter = Char;
		}
		Char->SetActorRotation(FRotator::ZeroRotator); // Reset whenever we possess, this is due to the AttachToComponent function when we possess a mount, it may ruin the character rotation. This should fix it.
		Char->SetActorTickEnabled(true);
	}

}

void AVeritexPlayerController::Server_KillPlayer_Implementation()
{

	if (MyCharacter != nullptr)
	{
		MyCharacter->Health = 0;
		MyCharacter->Food = 0;
		MyCharacter->Water = 0;
	}
	// PlayerInventoryComponent should now respawn the player.

}

bool AVeritexPlayerController::Server_KillPlayer_Validate()
{
	return true;
}


void AVeritexPlayerController::Suicide()
{
	Server_KillPlayer();
}

bool AVeritexPlayerController::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	Super::ProcessConsoleExec(Cmd, Ar, Executor);
	bool Result = false;
	APawn* const pawn = GetPawn();
	TArray<UActorComponent*> Components;
	pawn->GetComponents<UActorComponent>(Components);
	for (UActorComponent* Comp : Components)
	{
		if (IsValid(Comp))
		{
			Result = Comp->ProcessConsoleExec(Cmd, Ar, Executor) || Result;
		}
	}

	return Result;
}

void AVeritexPlayerController::PawnLeavingGame()
{
	Super::PawnLeavingGame(); // When we logout.
	// OnDestroy is when we either die, or logout.

	/* Default Inner Code of PawnLeavingGame() */
	/*if (GetPawn() != NULL)
	{
	GetPawn()->Destroy();
	SetPawn(NULL);
	}*/

}
