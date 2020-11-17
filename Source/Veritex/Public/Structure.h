// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/Actor.h"
#include "VeritexCharacter.h"
#include "VeritexStructs.h"
#include "Interfaces/WaterCraftInterface.h"
#include "Structure.generated.h"

class UDestructibleMesh;

UCLASS()
class VERITEX_API AStructure : public AActor, public IWaterCraftInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AStructure();

	virtual bool IsWaterCraft_Implementation() const override;
	virtual bool IsPossessable_Implementation() const override;

	virtual void PostNetInit() override;

	UFUNCTION(BlueprintPure)
		TMap<FString, FColor> GetColorMap();

	UPROPERTY(ReplicatedUsing = OnRep_PaintStructure, BlueprintReadWrite)
		FColor StructureColor;

	UPROPERTY(BlueprintReadWrite)
		UMaterialInstanceDynamic* DynamicMaterialInstance;

	UFUNCTION(BlueprintPure)
		UMaterialInstanceDynamic* CreateDynamicInstance();

	UFUNCTION()
		virtual void OnRep_PaintStructure();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = "Structures")
		TArray<FName> StructureOwners;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Structures)
		bool bHasSnapPoints = false;

	UFUNCTION(BlueprintCallable, Category = "Structures")
		float BoundingBoxTraces();

	UPROPERTY()
		USoundCue* DestroyedSound;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multicast_SpawnSoundOnDestruction();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multicast_SetDestructibleMeshOnDMActor(AVeritexDestructibleActor* VDA);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Structures)
		UDestructibleMesh* DM_Mesh;

	UFUNCTION(BlueprintNativeEvent, Category = "Hibernation")
		void OnHibernationModeChanged();

	UPROPERTY()
		bool bIsHibernating;

	UFUNCTION(BlueprintCallable, Category = "Hibernation")
		void SetHibernationMode(bool bHibernating);

	UFUNCTION(BlueprintPure, Category = "Hibernation")
		bool IsHibernationModeEnabled() { return bIsHibernating; }

	UPROPERTY(EditAnywhere, Category = Structures)
		bool ShouldCreateStorageTable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Fire)
		bool bShouldBurnLogs = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* Root;

	UPROPERTY(BlueprintReadOnly, Category = Database)
		FString DatabaseName = FString("Veritex");

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Structures)
		bool bIsDoorOpen = false;

	UPROPERTY(BlueprintReadWrite, Category = Storage)
		FString StorageItemsPath;

	UFUNCTION(BlueprintCallable, Category = Math)
		FHitResult& CalculateBoundsTrace();

	UPROPERTY()
		FHitResult CalcBoundsOutHitOne;

	UPROPERTY()
		FHitResult CalcBoundsOutHitTwo;

	UPROPERTY()
		FHitResult CalcBoundsOutHitThree;

	UPROPERTY()
		FHitResult CalcBoundsOutHitFour;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Storage)
		FString StorageItemsTableName; // Exists on Server but not clients, on clients its just an empty string

	//UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Replicated, Category = Structures)
		//int32 NeededTribeRank;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Structures)
		EStructureType StructureType; // Going to be used for validation purposes, checking structure type vs health

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Structures)
		EStructureQuality StructureQuality; // Going to be used for validation purposes, checking structure type vs health

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		int StructureMaxHealth = 0;

	//UPROPERTY(BlueprintReadOnly, Replicated, VisibleAnywhere, Category = Structures)
		//int DB_Key_Unique; // Used only when loading from the database, this is given to the AStructure class by AVeritexGameMode when spawning. This is a primary key in the "Structures" table, it auto-increments.
						   // Use this key for when you want to search the database for the structure for example: TakeDamage() => UpdateHealthInDB()

	UPROPERTY(BlueprintReadWrite, Category = Structures)
		FGuid UID;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Structures)
		FString OwnedBy; // Check a list of "Guilds" or "Tribes" for an existent name if it is already taken, don't allow permission.

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Structures)
		FString StructureName;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		int StructureHealth = StructureMaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		UStaticMeshComponent* Structure;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		AVeritexCharacter* VeritexCharacter; // Set in BPs when you spawn the structure

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		bool IsObstructed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		UMaterialInstanceDynamic* BaseMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Structures)
		float UpOffset;

	UFUNCTION(BlueprintPure, Category = DistanceTrace)
		float GetDistanceBetweenPoints(FVector A, FVector B);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

};
