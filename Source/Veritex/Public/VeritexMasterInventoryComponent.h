// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "VeritexStructs.h"

#include "VeritexMasterInventoryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class VERITEX_API UVeritexMasterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVeritexMasterInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Player Inventory Component")
		bool GetShouldCallOnInventoryReady() const;

	UFUNCTION(BlueprintCallable, Category = "Player Inventory Component")
		void SetShouldCallOnInventoryReady(bool NewValue);

	UFUNCTION()
		UArrayProperty* GetInventoryArray();

	UFUNCTION()
		TArray<FItemData> GetInventory();

	UFUNCTION(exec)
		void AddUserItem(const FString  & ItemID, int32 Count);

	UFUNCTION(Server, WithValidation, Reliable)
		void Server_AddUserItem(const FString  & ItemID, int32 Count);

	UFUNCTION(BlueprintImplementableEvent)
		void AddPlayerItem(const FString& ItemID, int32 Count, int32 Index);

	UFUNCTION(BlueprintImplementableEvent)
		void RemovePlayerItem(const FString& ItemID, int32 Count);

	UFUNCTION(BlueprintImplementableEvent)
		void EquipPlayerItem(const FString& ItemID, int32 Count);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
		void SetEquipmentArrayForCPP();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
		void SetInventoryArrayForCPP();

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
		TArray<FItemData> InventoryItems;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
		TArray<FItemData> GetInventoryItems();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
		TArray<FItemData> GetEquipmentItems();

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
		TArray<FItemData> EquipmentItems;

	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;

private:

	UPROPERTY()
		bool ShouldCallOnInventoryReady;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintReadWrite, Category = Cooking)
		FTimerHandle CookingTimer;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = Cooking)
		void StartCookingTimer(const FString& UncookedItemID, const FString& CookedItemID, float CookingTime);

	UFUNCTION(BlueprintImplementableEvent, Category = Cooking)
		void DoCookingTimerCheck(const FString& UncookedItemID, const FString& CookedItemID);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = InventoryComponentInfo)
		float MaxWeight = 100;

	UPROPERTY(EditAnywhere, Category = InventoryComponentInfo)
		bool HasUnlimitedWeight;

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool HasInfiniteWeight()
	{
		return HasUnlimitedWeight;
	}
};