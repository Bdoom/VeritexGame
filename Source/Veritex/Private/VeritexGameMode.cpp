// Daniel Gleason (C) 2017

#include "VeritexGameMode.h"
#include "Veritex.h"
#include "VeritexPlayerController.h"
#include "VeritexCharacter.h"
#include "VeritexFunctionLibrary.h"
#include "Structure.h"
#include "DBVeritexPlayer.h"

#include "VeritexAIController.h"

#include "Engine/LevelStreaming.h"
#include "VeritexWorldSettings.h"

#include "VeritexMasterInventoryComponent.h"
#include "EngineUtils.h"

#include "UObjectGlobals.h"

#include "JsonObjectConverter.h"

#include "Kismet/GameplayStatics.h"

#include "VeritexAsyncClasses.h"

#include "Item.h"

#include "BiomeSpawner.h"
#include "NPC.h"


#include "VeritexHUD.h"

//
//added "virtual" to RegisterTarget on UAISense_Sight.h this lets me override it in UAISense_VeritexSight.h
//virtual bool RegisterTarget(AActor& TargetActor, FQueriesOperationPostProcess PostProcess);
//

// I have made some changes to engine code for OnlineSessionAsyncServerSteam.h/cpp and SteamSessionKeys.h
// Modified OnlineSessionAsyncServerSteam.cpp to load ServerName= from [Script/Engine.Engine] in DefaultEngine.ini
// Using this sound for tree falling FX: https://www.youtube.com/watch?v=GfoUZJWKlyM

// WorldComposition.h needs to have the two methods set to public. 

// FoliageInstance->bReplicates = true must be true on the foliage instance component, and the AInstancedFoliageActor class, in order for foliage to work properly.

// Steam Ports (Required for hosting a server and having it show up on steam master server list).
// 8766
// 7777
// 27015
// 27016
// 13000
// Below Are not needed
// 27020
// 65375
// 50448
// If still not working, use program: currports or cPorts to check which ports the program is using.
// Can check easily on linux using grep.

// Inside of ACharacter, changed static unint8 SavedMovementMode; from static to not static (this is for swimming)
//
// Static variables for networking.
//
//uint8 SavedMovementMode;

//
// BodyInstance.h => Changed int16 InstanceBodyIndex to int32
//
/** Container for a physics representation of an object */
// USTRUCT()
// struct ENGINE_API FBodyInstance
// {
// 	GENERATED_USTRUCT_BODY()
// 
// 		/**
// 		*    Index of this BodyInstance within the SkeletalMeshComponent/PhysicsAsset.
// 		*    Is INDEX_NONE if a single body component
// 		*/
// 		int16 InstanceBodyIndex;

// Must have this in constructor of AInstancedFoliageActor
// bReplicates = true;
// bAlwaysRelevant = true;
// bReplicateMovement = false;
// bNetStartup = true;
// bNetLoadOnClient = true;
// NetUpdateFrequency = 0.5f;
// MinNetUpdateFrequency = 0.1f;
// NetPriority = 0.3f;
// NetDormancy = DORM_Initial;

// Event Driven Loader in Project Settings being turned on fixes this:  https://answers.unrealengine.com/questions/665092/starting-game-now-crashes-with-assertion-failed.html
// https://answers.unrealengine.com/questions/703852/compile-blueprint-crashes-editor-virtual-functions.html

// Adding OptimizeCode = CodeOptimization.Never; to Veritex.Build.cs file (testing to see if this helps with Unity Build exclusions when compiling)
//https://answers.unrealengine.com/questions/439700/compiled-c-code-does-not-execute.html

// Had to add this in 4.19 to get dedi servers to work. GetStaticMesh()->HasValidRenderData() sometimes returns false on dedi servers...

// void UHierarchicalInstancedStaticMeshComponent::BuildTreeAsync()
// {
// 	checkSlow(IsInGameThread());
// 
// 	// If we try to build the tree with the static mesh not fully loaded, we can end up in an inconsistent state which ends in a crash later
// 	checkSlow(!GetStaticMesh() || !GetStaticMesh()->HasAnyFlags(RF_NeedPostLoad));
// 
// 	check(!bIsAsyncBuilding);
// 
// 	// Verify that the mesh is valid before using it.
// 	const bool bMeshIsValid =
// 		// make sure we have instances
// 		PerInstanceSMData.Num() > 0 &&
// 		// make sure we have an actual staticmesh
// 		GetStaticMesh() &&
// 		(GetStaticMesh()->HasValidRenderData() || IsNetMode(NM_DedicatedServer)) &&
// 		// You really can't use hardware instancing on the consoles with multiple elements because they share the same index buffer. 
// 		// @todo: Level error or something to let LDs know this
// 		1;//GetStaticMesh()->LODModels(0).Elements.Num() == 1;

//static int32 MaxConnectionsToTickPerServerFrameAmount = 50;
//static FAutoConsoleVariable MaxConnectionsToTickPerServerFrame(TEXT("net.MaxConnectionsToTickPerServerFrame"), MaxConnectionsToTickPerServerFrameAmount, TEXT("When non-zero, the maximum number of channels that will have changed replicated to them per server update"));

void AVeritexGameMode::LoadUniverse()
{
	FString JsonString;
	if (FFileHelper::LoadFileToString(
		JsonString, *(UVeritexFunctionLibrary::PathToVeritexServerData() + FString("Universe.json"))))
	{
		JsonString = UVeritexFunctionLibrary::Decrypt(JsonString);
		FWorldData WorldData;
		//FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, WorldData.StaticStruct(), &WorldData, 0, 0);
		FJsonObjectConverter::JsonObjectStringToUStruct<FWorldData>(JsonString, &WorldData, 0, 0);
		for (FStructureData StructureData : WorldData.Structures)
		{
			FString StructurePath = StructureData.StructureClassPath;
			UClass* BPClass = LoadObject<UClass>(nullptr, (*StructurePath), nullptr, 0, nullptr);
			AStructure* SpawnedStructure = GetWorld()->SpawnActor<AStructure>(BPClass, StructureData.StructureLocation);
			if (SpawnedStructure)
			{
				UVeritexMasterInventoryComponent* InventoryComponent = SpawnedStructure->FindComponentByClass<
					UVeritexMasterInventoryComponent>();
				SpawnedStructure->StructureHealth = StructureData.Health;
				SpawnedStructure->StructureMaxHealth = StructureData.MaxHealth;
				//SpawnedStructure->NeededTribeRank = StructureData.NeededTribeRank;
				SpawnedStructure->OwnedBy = StructureData.OwnedBySteamID;
				SpawnedStructure->UID = StructureData.UID;
				SpawnedStructure->StructureOwners = StructureData.StructureOwners;
				SpawnedStructure->StructureColor = StructureData.StructureColor;

				if (InventoryComponent)
				{
					if (StructureData.StorageItems.Num() > 0)
					{
						int32 StorageItems = StructureData.StorageItems.Num();

						for (int i2 = 0; i2 < StructureData.StorageItems.Num(); i2++)
						{
							FItemData Item = StructureData.StorageItems[i2];
							InventoryComponent->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
						}
					}
				}
			}
		}

		for (FItemData Item : WorldData.Items)
		{
			AItem* ItemActor = GetWorld()->SpawnActor<AItem>(ItemClass, Item.ItemActorTransform);
			if (ItemActor)
			{
				ItemActor->ItemData = Item;
				ItemActor->OnCPPSpawned();
			}
		}

		// 		class ABiomeSpawner* Spawner;
		// 
		// 		for (TActorIterator<ABiomeSpawner> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		// 		{
		// 			if (*ActorItr)
		// 			{
		// 				Spawner = *ActorItr;
		// 			}
		// 		}
		// 
		// 		for (FNPCData NPCData : WorldData.NPCs)
		// 		{
		// 			UClass* NPCClass = LoadObject<UClass>(nullptr, (*NPCData.ClassPath), nullptr, 0, nullptr);
		// 			ANPC* NPC = GetWorld()->SpawnActor<ANPC>(NPCClass, NPCData.NPCTransform);
		// 
		// 			if (NPC)
		// 			{
		// 				NPC->SetStats();
		// 				NPC->Health = NPCData.Health;
		// 				NPC->MaxHealth = NPCData.MaxHealth;
		// 				NPC->OwnedBy = NPCData.OwnedBy;
		// 				for (FStructureData StructureData : NPCData.StructuresAttached)
		// 				{
		// 					UClass* BPClass = LoadObject<UClass>(nullptr, (*StructureData.StructureClassPath), nullptr, 0,
		// 					                                     nullptr);
		// 					AStructure* Structure = GetWorld()->SpawnActor<AStructure
		// 					>(BPClass, StructureData.StructureLocation);
		// 
		// 					if (!Structure)
		// 					{
		// 						continue;
		// 					}
		// 
		// 					Structure->SetOwner(NPC);
		// 
		// 					UVeritexMasterInventoryComponent* InventoryComponent = Structure->FindComponentByClass<
		// 						UVeritexMasterInventoryComponent>();
		// 					Structure->StructureHealth = StructureData.Health;
		// 					Structure->StructureMaxHealth = StructureData.MaxHealth;
		// 					Structure->OwnedBy = StructureData.OwnedBySteamID;
		// 					Structure->UID = StructureData.UID;
		// 					Structure->StructureOwners = StructureData.StructureOwners;
		// 
		// 					if (InventoryComponent)
		// 					{
		// 						if (StructureData.StorageItems.Num() > 0)
		// 						{
		// 							int32 StorageItems = StructureData.StorageItems.Num();
		// 
		// 							for (int i2 = 0; i2 < StructureData.StorageItems.Num(); i2++)
		// 							{
		// 								FItemData Item = StructureData.StorageItems[i2];
		// 								InventoryComponent->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
		// 							}
		// 						}
		// 					}
		// 
		// 					Structure->AttachToComponent(NPC->GetRootComponent(),
		// 					                             FAttachmentTransformRules::KeepWorldTransform);
		// 					NPC->StructuresAttached.Add(Structure);
		// 				}
		// 			}
		// 		}
	}
}

void AVeritexGameMode::UniverseLoaded(FWorldData const& WorldData)
{
	GLog->Log("Universe has been async loaded, beginning spawning of objects.");

	if (WorldData.Structures.Num() > 0)
	{
		for (int i = 0; i < WorldData.Structures.Num(); i++)
		{
			FStructureData StructureData = WorldData.Structures[i];
			FString StructurePath = StructureData.StructureClassPath;
			UClass* BPClass = LoadObject<UClass>(nullptr, (*StructurePath), nullptr, 0, nullptr);
			AStructure* SpawnedStructure = GetWorld()->SpawnActor<AStructure>(BPClass, StructureData.StructureLocation);
			UVeritexMasterInventoryComponent* InventoryComponent = SpawnedStructure->FindComponentByClass<
				UVeritexMasterInventoryComponent>();
			SpawnedStructure->StructureHealth = StructureData.Health;
			SpawnedStructure->StructureMaxHealth = StructureData.MaxHealth;
			//SpawnedStructure->NeededTribeRank = StructureData.NeededTribeRank;
			SpawnedStructure->OwnedBy = StructureData.OwnedBySteamID;
			SpawnedStructure->UID = StructureData.UID;

			if (InventoryComponent)
			{
				if (StructureData.StorageItems.Num() > 0)
				{
					for (int i2 = 0; i2 < StructureData.StorageItems.Num(); i2++)
					{
						FItemData Item = StructureData.StorageItems[i2];
						InventoryComponent->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
					}
				}
			}
		}
	}

	for (FNPCData NPCData : WorldData.NPCs)
	{
		UClass* NPCClass = LoadObject<UClass>(nullptr, (*NPCData.ClassPath), nullptr, 0, nullptr);
		ANPC* NPC = GetWorld()->SpawnActor<ANPC>(NPCClass, NPCData.NPCTransform);
		NPC->SetStats();
		NPC->Health = NPCData.Health;
		NPC->MaxHealth = NPCData.MaxHealth;
		NPC->OwnedBy = NPCData.OwnedBy;
		for (FStructureData StructureData : NPCData.StructuresAttached)
		{
			UClass* BPClass = LoadObject<UClass>(nullptr, (*StructureData.StructureClassPath), nullptr, 0, nullptr);
			AStructure* Structure = GetWorld()->SpawnActor<AStructure>(BPClass, StructureData.StructureLocation);
			Structure->SetOwner(NPC);

			UVeritexMasterInventoryComponent* InventoryComponent = Structure->FindComponentByClass<
				UVeritexMasterInventoryComponent>();
			Structure->StructureHealth = StructureData.Health;
			Structure->StructureMaxHealth = StructureData.MaxHealth;
			Structure->OwnedBy = StructureData.OwnedBySteamID;
			Structure->UID = StructureData.UID;
			Structure->StructureOwners = StructureData.StructureOwners;

			if (InventoryComponent)
			{
				if (StructureData.StorageItems.Num() > 0)
				{
					int32 StorageItems = StructureData.StorageItems.Num();

					for (int i2 = 0; i2 < StructureData.StorageItems.Num(); i2++)
					{
						FItemData Item = StructureData.StorageItems[i2];
						InventoryComponent->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
					}
				}
			}

			Structure->AttachToComponent(NPC->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			NPC->StructuresAttached.Add(Structure);
		}
	}

	for (FItemData Item : WorldData.Items)
	{
		AItem* ItemActor = GetWorld()->SpawnActor<AItem>(ItemClass, Item.ItemActorTransform);
		if (ItemActor)
		{
			ItemActor->ItemData = Item;
			ItemActor->OnCPPSpawned();
		}
	}


	// Converting Old Binary Data
	//FString OutputJsonString;
	//FJsonObjectConverter::UStructToJsonObjectString(FWorldData::StaticStruct(), &WorldData, OutputJsonString, 0, 0);
	//FFileHelper::SaveStringToFile(OutputJsonString, *(UVeritexFunctionLibrary::PathToVeritexServerData() + FString("Universe.json")));
}

AVeritexGameMode::AVeritexGameMode()
{
	// set default pawn class to our Blueprinted character
	// 	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/VeritexCharacter_BP"));
	// 	if (PlayerPawnBPClass.Class != NULL)
	// 	{
	// 		DefaultPawnClass = PlayerPawnBPClass.Class;
	// 	}

	// 	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassBP(TEXT("/Game/Blueprints/VeritexPlayerController_BP"));
	// 	if (PlayerControllerClassBP.Class != NULL)
	// 	{
	// 		PlayerControllerClass = PlayerControllerClassBP.Class;
	// 	}
	// 	static ConstructorHelpers::FClassFinder<AHUD> HUDClassBP(TEXT("/Game/Blueprints/VeritexHUD_BP"));
	// 
	// 	if (HUDClassBP.Class != NULL)
	// 	{
	// 		HUDClass = HUDClassBP.Class;
	// 	}
	// 
	// 	static ConstructorHelpers::FClassFinder<AGameState> GameStateBP(TEXT("/Game/Blueprints/VeritexGameState_BP"));
	// 	if (GameStateBP.Class != NULL)
	// 	{
	// 		GameStateClass = GameStateBP.Class;
	// 	}

	// 	PlayerStateClass = AVeritexPlayerState::StaticClass();
	// 
	// 	GameSessionClass = AVeritexGameSession::StaticClass();
}

AVeritexGameMode::FWorldDataLoaded AVeritexGameMode::WorldDataLoaded;

void AVeritexGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update player locations
	PlayerLocations.Empty();
	for (TActorIterator<AVeritexCharacter> VeritexChar(GetWorld()); VeritexChar; ++VeritexChar)
	{
		AVeritexCharacter* Char = *VeritexChar;
		if (IsValid(Char))
		{
			PlayerLocations.Add(Char->GetActorLocation());
		}
	}

	// 	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	// 	{
	// 		class AVeritexPlayerController* const PC = Cast<AVeritexPlayerController>((*Iterator).Get());
	// 		if (PC)
	// 		{
	// 			if (PC->MyCharacter)
	// 			{
	// 				PlayerLocations.Add(PC->MyCharacter->GetActorLocation());
	// 			}
	// 		}
	// 	}

	ActiveAINum = 0;
	StasisAINum = 0;


	// Update AI stasis state
	for (class AVeritexAIController* const It : AIControllers)
	{
		if (It->GetPawn() && It->GetPawn()->HasActorBegunPlay() && It->GetPawn()->GetGameTimeSinceCreation() > 1.f)
		{
			const bool bIsInRadius = IsLocationNearbyPlayer(It->GetPawn()->GetActorLocation(), StasisDistance);
			//10000.f); // 30000.f
			//It->SetHibernate(!bIsInRadius);

			ANPC* NPC = Cast<ANPC>(It->GetPawn());
			if (NPC)
			{
				if (!bIsInRadius)
				{
					NPC->ReturnedToPool();
				}
				else
				{
					NPC->ReleasedFromPool(FTransform::Identity);
				}
			}

			// 			if (It->IsHibernationModeEnabled())
			// 			{
			// 				++StasisAINum;
			// 			}
			// 			else
			// 			{
			// 				++ActiveAINum;
			// 			}
		}
	}

	//GLog->Log(FString("Stasis AI: ") + FString::FromInt(StasisAINum) + FString(" ActiveAI : ") + FString::FromInt(ActiveAINum));
}

void AVeritexGameMode::StartSpoilageTimer(const FString& ItemID, float SpoilageTimer, UActorComponent* StorageComponent)
{
	TWeakObjectPtr<UActorComponent> MyStorageComponent = StorageComponent;
	TWeakObjectPtr<AVeritexGameMode> WeakThis{ this };

	FTimerHandle SpoilageTimerHandle;
	FTimerDelegate SpoilageTimerDelegate;

	if (MyStorageComponent.IsValid() && !MyStorageComponent.IsStale())
	{
		SpoilageTimerDelegate.BindLambda([WeakThis, MyStorageComponent, ItemID]()
		{
			if (WeakThis.IsValid() && !WeakThis.IsStale() && MyStorageComponent.IsValid() && !MyStorageComponent.
				IsStale())
			{
				WeakThis->SpoilItem(ItemID, MyStorageComponent.Get());
			}
		});
	}
	GetWorld()->GetTimerManager().SetTimer(SpoilageTimerHandle, SpoilageTimerDelegate, SpoilageTimer, false);
	SpoilageTimers.Add(ItemID, SpoilageTimerHandle);
}

bool AVeritexGameMode::PlayingInEditor()
{
	return GetWorld()->IsPlayInEditor();
}

AVeritexCharacter* AVeritexGameMode::GetPlayerByUniqueSteamID(FString UniqueSteamID)
{
	if (Players.Contains(UniqueSteamID))
	{
		return Players[UniqueSteamID];
	}
	for (TActorIterator<AVeritexCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AVeritexCharacter* VeritexChar = Cast<AVeritexCharacter>(*ActorItr);
		TWeakObjectPtr<AVeritexCharacter> VeritexCharacter = VeritexChar;
		if (VeritexCharacter.IsValid())
		{
			if (VeritexChar)
			{
				if (VeritexChar->UniqueSteamID == UniqueSteamID)
				{
					return VeritexChar;
				}
			}
		}
	}

	return nullptr;
}

bool AVeritexGameMode::IsDedicatedServer()
{
	return IsNetMode(NM_DedicatedServer);
}

void AVeritexGameMode::PostLoad()
{
	Super::PostLoad();
}

void AVeritexGameMode::PostLogin(APlayerController* NewPlayer)
{
	AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(NewPlayer);
	if (VPC)
	{
		VPC->Client_TryLoadUser();
	}

	Super::PostLogin(NewPlayer);
}

void AVeritexGameMode::BeginPlay()
{
	Super::BeginPlay();

	//OnCharacterDataLoaded_Delegate.BindUObject(this, &AVeritexGameMode::OnCharacterDataLoaded);
	// 	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GetWorld());
	// 	FoliageActor->SetReplicates(true);
	// 	TArray<class UVeritexFoliageInstance*> FoliageComps;
	// 	FoliageActor->GetComponents<UVeritexFoliageInstance>(FoliageComps);
	// 
	// 	for (class UVeritexFoliageInstance* Comp : FoliageComps)
	// 	{
	// 		for (int i = 0; i < Comp->GetNumInstances(); ++i)
	// 		{
	// 			FVeritexFoliageDataSet FoliageInfo;
	// 			FoliageInfo.FoliageInstanceComponent = Comp;
	// 			FoliageInfo.Index = i;
	// 			FTransform InstanceTransform;
	// 			Comp->GetInstanceTransform_public(i, InstanceTransform);
	// 			FoliageInfo.Transform = InstanceTransform;
	// 			FoliageData.Add(FoliageInfo);
	// 		}
	// 	}


	//GetWorld()->SpawnActor<AVeritexFoliageManager>(AVeritexFoliageManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);


	LoadUniverse();
	GetWorldTimerManager().SetTimer(SaveWorldTimerHandle, this, &AVeritexGameMode::SaveWorld, 10.f, true);
	// Setup Timer to Save World Every 300 Seconds (5 Minutes) (3600.f = 1 hour)
}

//AVeritexGameMode::FCharacterDataLoaded AVeritexGameMode::OnCharacterDataLoaded_Delegate;
// 
// void AVeritexGameMode::OnCharacterDataLoaded(FCharacterData const& Character, bool bKeepAlive, const FVector SpawnPoint)
// {
// 	FString ClassPath = "/Game/Blueprints/VeritexCharacter_BP.VeritexCharacter_BP_C";
// 	UClass* CharacterClass = LoadObject<UClass>(nullptr, (*ClassPath), nullptr, 0, nullptr);
// 
// 	AVeritexCharacter* Char;
// 
// 	FTransform SpawnTransform;
// 	if (!SpawnPoint.IsNearlyZero())
// 	{
// 		SpawnTransform.SetLocation(SpawnPoint);
// 	}
// 	else
// 	{
// 		SpawnTransform = Character.PlayerPosition;
// 	}
// 
// 	if (!IsDedicatedServer())
// 	{
// 		GetWorld()->SpawnActor<AActor>(LoadingBlockerBP_Class, SpawnTransform);
// 	}
// 
// 	if (GetWorld()->WorldComposition)
// 	{
// 		GetWorld()->WorldComposition->UpdateStreamingState(SpawnTransform.GetLocation());
// 		TArray<FDistanceVisibleLevel> OutVisibleLevels;
// 		TArray<FDistanceVisibleLevel> OutHiddenLevels;
// 		GetWorld()->WorldComposition->GetDistanceVisibleLevels(SpawnTransform.GetLocation(), OutVisibleLevels, OutHiddenLevels);
// 
// 		for (FDistanceVisibleLevel LevelTile : OutVisibleLevels)
// 		{
// 			LevelTile.StreamingLevel->bDisableDistanceStreaming = true;
// 			LevelTile.StreamingLevel->SetShouldBeLoaded(true);
// 			LevelTile.StreamingLevel->SetShouldBeVisible(true);
// 		}
// 	}
// 
// 	Char = GetWorld()->SpawnActor<AVeritexCharacter>(CharacterClass, SpawnTransform);
// 
// 	UVeritexMasterInventoryComponent* Inventory = Char->FindComponentByClass<UVeritexMasterInventoryComponent>();
// 	if (bKeepAlive)
// 	{
// 		Char->Health = Character.MaxHealth;
// 	}
// 	else
// 	{
// 		Char->Health = Character.Health;
// 	}
// 
// 	Char->MaxHealth = Character.MaxHealth;
// 	if (bKeepAlive)
// 	{
// 		Char->Food = Character.MaxFood;
// 	}
// 	else
// 	{
// 		Char->Food = Character.Food;
// 	}
// 	Char->CurrentLevel = Character.Level;
// 	Char->Experience = Character.Experience;
// 	if (Inventory)
// 	{
// 		for (FItemData Item : Character.Inventory)
// 		{
// 			Inventory->AddPlayerItem(Item.ItemID, Item.Count, Item.Index);
// 		}
// 		for (FItemData EquipmentItem : Character.Equipment)
// 		{
// 			Inventory->EquipPlayerItem(EquipmentItem.ItemID, EquipmentItem.Count);
// 		}
// 		Inventory->MaxWeight = Character.MaxWeight;
// 		Char->MaxStamina = Character.MaxStamina;
// 	}
// 	Char->LevelUpPoints = Character.LevelUpPoints;
// 	Char->MaxFood = Character.MaxFood;
// 	Char->MaxOxygen = Character.MaxOxygen;
// 	Char->MaxWater = Character.MaxWater;
// 	Char->MeleeDamage = Character.MeleeDamage;
// 	Char->MovementSpeed = Character.MovementSpeed;
// 
// 	Char->UniqueSteamID = Character.UniqueSteamID;
// 
// 	if (bKeepAlive)
// 	{
// 		Char->Water = Character.MaxWater;
// 	}
// 	else
// 	{
// 		Char->Water = Character.Water;
// 	}
// 
// 	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
// 	{
// 		AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(Iterator->Get());
// 		if (VPC)
// 		{
// 			if (VPC->UniqueSteamID.Equals(Character.UniqueSteamID))
// 			{
// 				if (Char->Health <= 0)
// 				{
// 					VPC->Client_OpenWorldMapWidget();
// 					if (!Char->IsPendingKill())
// 					{
// 						Char->Destroy();
// 					}
// 					return;
// 				}
// 
// 				VPC->Possess(Char);
// 				VPC->MyCharacter = Char;
// 			}
// 		}
// 	}
// 
// }

bool AVeritexGameMode::IsLocationNearbyPlayer(const FVector& Location, const float Radius) const
{
	for (const FVector& It : PlayerLocations)
	{
		const float Dist = (It - Location).Size();
		if (Dist <= Radius)
		{
			return true;
		}
	}
	return false;
}

AVeritexWorldSettings* AVeritexGameMode::GetWorldSettings()
{
	return Cast<AVeritexWorldSettings>(GetWorld()->GetWorldSettings());
}

void AVeritexGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void AVeritexGameMode::SaveWorld()
{
	FWorldData WorldData;

	for (TActorIterator<AStructure> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AStructure* Structure = *ActorItr;
		UVeritexMasterInventoryComponent* InventoryComponent = Structure->FindComponentByClass<
			UVeritexMasterInventoryComponent>();
		if (Structure)
		{
			if (Structure->GetName().Contains("Placeholder"))
			{
				continue;
			}

			AActor* StructureOwner = Structure->GetOwner();
			if (StructureOwner && StructureOwner->IsA(StaticClass()))
			{
				continue;
				// Will be saved when we loop through NPCs, these are most likely owned by Rafts or other vehicles. 
			}

			FStructureData StructureData;
			StructureData.Health = Structure->StructureHealth;
			StructureData.MaxHealth = Structure->StructureMaxHealth;
			//StructureData.NeededTribeRank = Structure->NeededTribeRank;
			StructureData.StructureClassPath = Structure->GetClass()->GetPathName();
			StructureData.StructureLocation = Structure->GetActorTransform();
			StructureData.UID = Structure->UID;
			StructureData.OwnedBySteamID = Structure->OwnedBy;
			StructureData.StructureColor = Structure->StructureColor;
			if (Structure->StructureOwners.Contains(FName(TEXT("None"))))
			{
				Structure->StructureOwners.Remove(FName(TEXT("None")));
			}

			StructureData.StructureOwners = Structure->StructureOwners;
			if (InventoryComponent)
			{
				StructureData.StorageItems = InventoryComponent->GetInventory();
			}

			WorldData.Structures.Add(StructureData);
		}
	}

	for (TActorIterator<ANPC> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ANPC* NPC = *ActorItr;
		if (!NPC->GetName().Contains("Raft")) // Only save rafts for now!
		{
			continue;
		}

		if (NPC)
		{
			FNPCData NPCData;
			FString NPCClassPath = NPC->GetClass()->GetPathName();
			NPCData.ClassPath = NPCClassPath;
			NPCData.Health = NPC->Health;
			NPCData.MaxHealth = NPC->MaxHealth;
			NPCData.NPCTransform = NPC->GetActorTransform();
			NPCData.OwnedBy = NPC->OwnedBy;
			TArray<AStructure*> NullStructures;
			for (AStructure* Structure : NPC->StructuresAttached)
			{
				if (!Structure)
				{
					NullStructures.Add(Structure);
					continue;
				}


				UVeritexMasterInventoryComponent* InventoryComponent = Structure->FindComponentByClass<
					UVeritexMasterInventoryComponent>();
				FStructureData StructureData;
				StructureData.Health = Structure->StructureHealth;
				StructureData.MaxHealth = Structure->StructureMaxHealth;
				StructureData.StructureClassPath = Structure->GetClass()->GetPathName();
				StructureData.StructureLocation = Structure->GetActorTransform();
				StructureData.UID = Structure->UID;
				StructureData.OwnedBySteamID = Structure->OwnedBy;
				if (Structure->StructureOwners.Contains(FName(TEXT("None"))))
				{
					Structure->StructureOwners.Remove(FName(TEXT("None")));
				}

				StructureData.StructureOwners = Structure->StructureOwners;
				if (InventoryComponent)
				{
					StructureData.StorageItems = InventoryComponent->GetInventory();
				}
				NPCData.StructuresAttached.Add(StructureData);
			}

			for (AStructure* NullStructure : NullStructures)
			{
				NPC->StructuresAttached.Remove(NullStructure);
			}
			//NPCData.StructuresAttached = 
			WorldData.NPCs.Add(NPCData);
		}
	}

	for (TActorIterator<AItem> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AItem* Item = *ActorItr;
		if (Item)
		{
			Item->SetItemData();
			FItemData ItemData = Item->ItemData;
			if (!ItemData.ItemID.IsEmpty())
			{
				ItemData.ItemActorTransform = Item->GetActorTransform();
				WorldData.Items.Add(ItemData);
			}
		}
	}

	(new FAutoDeleteAsyncTask<FSaveLoadWorldTask>(true, WorldData))->StartBackgroundTask();
}

void AVeritexGameMode::Logout(AController* ExitingPlayer)
{
	AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(ExitingPlayer);
	if (VPC)
	{
		VPC->SavePlayer(true);
	}

	Super::Logout(ExitingPlayer);
}

AActor* AVeritexGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (Player)
	{
		AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(Player);
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), SpawnLocations);
		// Using a custom class because we may have biome spawns later on for spawning. So we would need to add variables onto this class and check what biome they are based on that variable.


		if (SpawnLocations.Num() > 0)
		{
			int32 randSpawn = FMath::RandRange(0, SpawnLocations.Num() - 1);
			return SpawnLocations[randSpawn];
		}
	}


	return nullptr;
}

// If this returns false it will call ChoosePlayerStart_Implementation() and use that as the player start
// If returns true it will use the StartSpot stored on Player instead
bool AVeritexGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}
