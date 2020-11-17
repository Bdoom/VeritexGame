// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "DestructibleMesh.h"
#include "VeritexFoliageInstance.generated.h"

class USoundCue;


USTRUCT(BlueprintType)
struct FVeritexFoliageInfo
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		int32 IndexID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		int32 AmountOfResource;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		bool bIsDespawned = false;


	void TakeResource(int32 AmountToRemove)
	{
		AmountOfResource -= AmountToRemove;
	}

	void AddResource(int32 AmountToAdd)
	{
		AmountOfResource += AmountToAdd;
	}

	void SetResourceAmount(int32 Amount)
	{
		AmountOfResource = Amount;
	}

};

UCLASS()
class VERITEX_API UVeritexFoliageInstance : public UFoliageInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	int32 GetNumInstances();
	void GetInstanceTransform_public(int32 Index, FTransform& OutTransform);

public:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void PopulateInstanceMap();

	UFUNCTION(BlueprintCallable, Category = "Foliage")
		void EnableInstanceCollision(int32 InstanceIndex);

	UFUNCTION(BlueprintCallable, Category = "Foliage")
		void DisableInstanceCollision(int32 InstanceIndex);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foliage")
		bool IsBushFoliage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FoliageSounds)
		USoundCue* SoundToPlay;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		UDestructibleMesh* DestructibleMesh;

	UPROPERTY()
		bool bFoliageSetup;

	UFUNCTION(BlueprintCallable, Category = Foliage)
		bool IsDespawned(int32 InstanceIndex);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Unreliable)
		void Server_ResetFoliageInfo(int32 Item);

	UFUNCTION(BlueprintCallable, Category = Foliage)
		FVeritexFoliageInfo& GetFoliageInfoByID(int32 Item);

	UFUNCTION(BlueprintCallable, Category = Foliage)
		bool IsFoliageDespawned(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Foliage)
		void SetFoliageInstanceDespawned(bool bShouldBeDespawned, int32 Item);

	/************************************************************************/
	/* Must be called on server                                             */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = Foliage)
		void SetFoliageAmountOfResource(const int32 AmountToRemove, const int32 item, const FString& LeftHandItemName, int32& AmountToAddToInventory, FString& ItemToAddName, bool bOverrideNoWeapon);

	/************************************************************************/
	/* Must be called on server, otherwise it will return nullptr issue.    */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = Foliage)
		int32 GetFoliageAmount(const int32 item);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		float RespawnTimeInSeconds = 0;

	UFUNCTION(BlueprintCallable, Category = Foliage)
		void FoliageTimer(int32 Item, FVector OriginalLocation, FRotator OriginalRotation);

	UFUNCTION(BlueprintCallable, Category = Foliage)
		void StartNewTimer(int32 Item, FVector OriginalLocation, FRotator OriginalRotation);

	UFUNCTION(BlueprintCallable)
		void ResetFoliageInfo(int32 Item);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
		void Server_DespawnFoliage(int32 Item);

	UFUNCTION(BlueprintCallable, NetMulticast, WithValidation, Reliable)
		void Multicast_RespawnFoliage(int32 Item, FVector OriginalLocation, FRotator OriginalRotation);

	UFUNCTION(BlueprintCallable, NetMulticast, WithValidation, Reliable)
		void Multicast_DespawnFoliage(int32 Item);

	UPROPERTY(BlueprintReadWrite, Category = Foliage)
		TMap<int32, FVeritexFoliageInfo> FoliageInfos;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		int32 MinValueOfResource = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		int32 MaxValueOfResource = 0;

	//UFUNCTION(BlueprintPure, Category = Foliage) - Was supported by blueprint in 4.16, but not in 4.17?
	TArray<FInstancedStaticMeshInstanceData> GetInstances();

	UFUNCTION(BlueprintPure, Category = Foliage)
		TArray<int32> GetInstanceIDs();

	UFUNCTION(BlueprintCallable, Category = Foliage)
		void UpdateRenderState();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		TArray<FName> ResourcesToGive;

	UVeritexFoliageInstance();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Foliage)
		float ExperienceToGive;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	virtual bool UpdateInstanceTransform(int32 InstanceIndex, const FTransform& NewInstanceTransform, bool bWorldSpace, bool bMarkRenderStateDirty = false, bool bTeleport = false) override;


	virtual void PostLoad() override;

protected:
	virtual void OnCreatePhysicsState() override;

};
