// Daniel Gleason (C) 2017

#include "VeritexFoliageInstance.h"
#include "Veritex.h"
#include "TimerManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Private/InstancedStaticMesh.h"
#include "Sound/SoundCue.h"


UVeritexFoliageInstance::UVeritexFoliageInstance()
{
	bReplicates = true; // Instance + AInstancedFoliageActor.cpp must also be set to bReplicates = true;

	//UseDynamicInstanceBuffer = true;
	//KeepInstanceBufferCPUAccess = true;

	static ConstructorHelpers::FObjectFinder<USoundCue> RockSmashCue(TEXT("SoundCue'/Game/Sounds/ProSoundCollection_v1_3_MONO/Impacts_Smashable/Cues/RockSmashCue.RockSmashCue'"));
	static ConstructorHelpers::FObjectFinder<USoundCue> TreeHitSoundCue(TEXT("SoundCue'/Game/Sounds/UniversalSoundFX/Sound_Effects/IMPACTS/Stone/TreeHit.TreeHit'"));

	PrimaryComponentTick.bCanEverTick = false;
	SetGenerateOverlapEvents(false);

	//BodyInstance.bUseAsyncScene = true;

	SetCollisionProfileName(FName("Foliage"));
}

void UVeritexFoliageInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UVeritexFoliageInstance::UpdateInstanceTransform(int32 InstanceIndex, const FTransform& NewInstanceTransform, bool bWorldSpace, bool bMarkRenderStateDirty /*= false*/, bool bTeleport /*= false*/)
{
	//const bool bOldPhysicsStateCreated = bPhysicsStateCreated;

	//bPhysicsStateCreated = false;

	const bool bResult = Super::UpdateInstanceTransform(InstanceIndex, NewInstanceTransform, bWorldSpace, bMarkRenderStateDirty, bTeleport);

	//bPhysicsStateCreated = bOldPhysicsStateCreated;

	return bResult;
}

void UVeritexFoliageInstance::PostLoad()
{
	Super::PostLoad();
	Mobility = EComponentMobility::Static;
	SetGenerateOverlapEvents(false);

	// 	if (PerInstanceRenderData)
	// 	{
	// 		PerInstanceRenderData->InstanceBuffer.RequireCPUAccess = true;
	// 	}

}

void UVeritexFoliageInstance::OnCreatePhysicsState()
{
	Super::OnCreatePhysicsState();
	// 	check(InstanceBodies.Num() == 0);
	// 
	// 	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
	// 
	// 	if (!PhysScene)
	// 	{
	// 		return;
	// 	}
	// 
	// 	// Create all the bodies.
	// 	//CreateAllInstanceBodies();
	// 
	// 	USceneComponent::OnCreatePhysicsState();

}

int32 UVeritexFoliageInstance::GetNumInstances()
{
	return GetInstanceCount();
}

void UVeritexFoliageInstance::GetInstanceTransform_public(int32 Index, FTransform& OutTransform)
{
	GetInstanceTransform(Index, OutTransform);
}

void UVeritexFoliageInstance::BeginPlay()
{
	// Do BOX trace to hide the foliage instance
	// Make sure this isn't being called more than once
	Super::BeginPlay();

	SetCollisionProfileName(FName("Foliage"));

	PopulateInstanceMap();
}

void UVeritexFoliageInstance::PopulateInstanceMap()
{
	//if (GetOwner()->HasAuthority())
	//{
		//if (!bFoliageSetup)
		//{
		FoliageInfos.Empty();
		for (int32 key = 0; key < GetInstanceCount(); key++)
		{
			FVeritexFoliageInfo Info;
			Info.IndexID = key;
			if (MinValueOfResource > 0 && MaxValueOfResource > 0)
			{
				Info.AmountOfResource = FMath::RandRange(MinValueOfResource, MaxValueOfResource);
			}
			else
			{
				Info.AmountOfResource = FMath::RandRange(10, 50);
			}

			FoliageInfos.Add(key, Info);
		}

		//bFoliageSetup = true;
	//}
	//}
}

void UVeritexFoliageInstance::Multicast_RespawnFoliage_Implementation(int32 Item, FVector OriginalLocation, FRotator OriginalRotation)
{
	FTransform OutTransform;
	GetInstanceTransform(Item, OutTransform);

	FTransform NewTransform = FTransform(OriginalRotation, OriginalLocation, OutTransform.GetScale3D());

	UpdateInstanceTransform(Item, NewTransform, false, true, true);

}

bool UVeritexFoliageInstance::Multicast_RespawnFoliage_Validate(int32 Item, FVector OriginalLocation, FRotator OriginalRotation)
{
	return true;
}

void UVeritexFoliageInstance::Multicast_DespawnFoliage_Implementation(int32 Item)
{
	FTransform OutTransform;
	GetInstanceTransform(Item, OutTransform);

	FVector Location = OutTransform.GetLocation() - FVector(0, 0, 100000);
	FQuat Rotation = OutTransform.GetRotation();
	FVector Scale = OutTransform.GetScale3D();

	FTransform NewTransform = FTransform(Rotation, Location, Scale);
	UpdateInstanceTransform(Item, NewTransform, false, true, true);

}

bool UVeritexFoliageInstance::Multicast_DespawnFoliage_Validate(int32 Item)
{
	return true;
}

void UVeritexFoliageInstance::ResetFoliageInfo(int32 Item)
{
	if (GetOwner()->HasAuthority())
	{
		FVeritexFoliageInfo& Info = FoliageInfos[Item];

		if (MinValueOfResource > 0 && MaxValueOfResource > 0)
		{
			Info.AmountOfResource = FMath::RandRange(MinValueOfResource, MaxValueOfResource);
		}
		else
		{
			Info.AmountOfResource = FMath::RandRange(5, 15);
		}

		Info.bIsDespawned = false;
	}
	else
	{
		Server_ResetFoliageInfo(Item);
	}
}

void UVeritexFoliageInstance::Server_ResetFoliageInfo_Implementation(int32 Item)
{
	ResetFoliageInfo(Item);
}

bool UVeritexFoliageInstance::Server_ResetFoliageInfo_Validate(int32 Item)
{
	return true;
}

void UVeritexFoliageInstance::Server_DespawnFoliage_Implementation(int32 Item)
{
	FoliageInfos[Item].bIsDespawned = true;

	FTransform OutTransform;
	GetInstanceTransform(Item, OutTransform);

	FVector Location = OutTransform.GetLocation();
	FQuat Rotation = OutTransform.GetRotation();
	FVector Scale = FVector(0, 0, 0);

	FTransform NewTransform = FTransform(Rotation, Location, Scale);

	UpdateInstanceTransform(Item, NewTransform, false, true, true);
}

bool UVeritexFoliageInstance::Server_DespawnFoliage_Validate(int32 Item)
{
	return true;
}

TArray<FInstancedStaticMeshInstanceData> UVeritexFoliageInstance::GetInstances()
{
	return PerInstanceSMData;
}

TArray<int32> UVeritexFoliageInstance::GetInstanceIDs()
{
	return InstanceReorderTable;
}

void UVeritexFoliageInstance::UpdateRenderState()
{
	MarkRenderStateDirty();
}


int32 UVeritexFoliageInstance::GetFoliageAmount(const int32 item)
{
	if (FoliageInfos.Contains(item))
	{
		FVeritexFoliageInfo& Info = FoliageInfos[item];

		return Info.AmountOfResource;
	}
	else
	{
		//check(item > 0 && "Make sure FBodyInstance->InstanceBodyIndex is not int16 (requires custom engine build), and is instead int32 or higher. (This causes negative valued instance ids) Located in Engine->Source->Runtime->Engine->Classes->PhysicsEngine->BodyInstance.h");
		return 0;
	}
}

void UVeritexFoliageInstance::FoliageTimer(int32 Item, FVector OriginalLocation, FRotator OriginalRotation)
{
	Multicast_RespawnFoliage(Item, OriginalLocation, OriginalRotation);
	ResetFoliageInfo(Item);
}

void UVeritexFoliageInstance::StartNewTimer(int32 Item, FVector OriginalLocation, FRotator OriginalRotation)
{
	FTimerDelegate TimerDel;

	FTimerHandle TimerHandle;

	TimerDel.BindUFunction(this, FName("FoliageTimer"), Item, OriginalLocation, OriginalRotation);

	if (RespawnTimeInSeconds == 0)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1000.f, false); // Default Respawn Timer is 30 minutes 1800.f // 1000.f
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, RespawnTimeInSeconds, false);
	}
}

bool UVeritexFoliageInstance::IsDespawned(int32 InstanceIndex)
{
	return FoliageInfos[InstanceIndex].bIsDespawned;
}

FVeritexFoliageInfo& UVeritexFoliageInstance::GetFoliageInfoByID(int32 Item)
{
	return FoliageInfos[Item];
}

bool UVeritexFoliageInstance::IsFoliageDespawned(int32 Index)
{
	FVeritexFoliageInfo& Info = FoliageInfos[Index];
	if (Info.bIsDespawned)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UVeritexFoliageInstance::SetFoliageInstanceDespawned(bool bShouldBeDespawned, int32 Item)
{
	if (FoliageInfos.Num() > 0)
	{
		FVeritexFoliageInfo& Info = FoliageInfos[Item];
		Info.bIsDespawned = bShouldBeDespawned;
	}
}

void UVeritexFoliageInstance::SetFoliageAmountOfResource(const int32 AmountToRemove, const int32 item, const FString& LeftHandItemName, int32& AmountToAddToInventory, FString& ItemToAddName, bool bOverrideNoWeapon)
{
	FVeritexFoliageInfo& Info = FoliageInfos[item];

	AmountToAddToInventory = AmountToRemove;
	Info.TakeResource(AmountToRemove);

	int32 RandomResourceToGive = FMath::RandRange(0, ResourcesToGive.Num() - 1);

	ItemToAddName = ResourcesToGive[RandomResourceToGive].ToString();

	// Handle Weapon Buffs, different weapons can give us more of a specific resource.
	if (LeftHandItemName == "Hatchet")
	{
		if (ItemToAddName.Equals("Thatch") || ItemToAddName.Equals("WoodLogs"))
		{
			AmountToAddToInventory *= 5;
		}

		// Temporary Fix for RNG with IronOre from all rocks (Really shitty fix, and completely stupid... just ignore this idiotic concept lol)
		if (ItemToAddName.Equals("Flint") || ItemToAddName.Equals("Stone"))
		{
			int32 RandomChanceOfIronOre = FMath::RandRange(1, 10);
			if (RandomChanceOfIronOre == 3)
			{
				ItemToAddName = "IronOre";
			}
		}
		// End Temp Fix
	}

	if (LeftHandItemName == "Pickaxe")
	{
		if (ItemToAddName.Equals("Stone") || ItemToAddName.Equals("Flint") || ItemToAddName.Contains("Ore"))
		{
			AmountToAddToInventory *= 5;
		}
	}

	if (LeftHandItemName == "None" && !bOverrideNoWeapon)
	{
		if (ItemToAddName.Equals("Stone") || ItemToAddName.Equals("Flint") || ItemToAddName.Contains("Ore") || ItemToAddName.Contains("Iron"))
		{
			AmountToAddToInventory = 0;
			return;
		}
	}

}

void UVeritexFoliageInstance::EnableInstanceCollision(int32 InstanceIndex)
{
	if (InstanceIndex >= InstanceBodies.Num())
	{
		InstanceBodies.SetNumZeroed(InstanceIndex + 1, false);
	}

	FBodyInstance*& InstanceBodyInstance = InstanceBodies[InstanceIndex];

	if (InstanceBodyInstance && InstanceBodyInstance->IsValidBodyInstance())
	{
		FTransform InstanceTransform = FTransform::Identity;
		GetInstanceTransform(InstanceIndex, InstanceTransform, true);
		// Update existing BodyInstance
		InstanceBodyInstance->SetBodyTransform(InstanceTransform, TeleportFlagToEnum(true));
		InstanceBodyInstance->UpdateBodyScale(InstanceTransform.GetScale3D());
	}
	else
	{
		// create new BodyInstance
		InstanceBodyInstance = new FBodyInstance();
		InitInstanceBody(InstanceIndex, InstanceBodyInstance);
	}
}

void UVeritexFoliageInstance::DisableInstanceCollision(int32 InstanceIndex)
{
	if (InstanceBodies.IsValidIndex(InstanceIndex))
	{
		FBodyInstance*& InstanceBodyInstance = InstanceBodies[InstanceIndex];
		if (InstanceBodyInstance && InstanceBodyInstance->IsValidBodyInstance())
		{
			//InstanceBodyInstance->bUseAsyncScene = true;
			// delete BodyInstance
			InstanceBodyInstance->TermBody();
			delete InstanceBodyInstance;
			InstanceBodyInstance = nullptr;
		}
	}
}
