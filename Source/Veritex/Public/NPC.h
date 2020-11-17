
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NavigationInvokerComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "VeritexCharacter.h"
#include "Interfaces/WaterCraftInterface.h"

#include "NPC.generated.h"

UENUM(BlueprintType)
enum class ENPCType : uint8
{
	Herbivore,
	Omnivore,
	Carnivore
};

UENUM(BlueprintType)
enum class EMountName : uint8
{
	Phoenix,
	Griffin,
	SkyWhale,
	Wolf,
	Bear,
	Tiger,
	LochNess,
	Pegasus,
	Penguin,
	Deer,
	Elephant,
	Dragon,
	Lion,
	Boar,
	Cat,
	Dog,
	Pig,
	Chicken,
	Fox,
	Cow,
	Crocodile,
	Goat,
	Rabbit,
	Sheep,
	Raft

};

UCLASS()
class VERITEX_API ANPC : public ACharacter, public IWaterCraftInterface
{
	GENERATED_BODY()

public:

	ANPC(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite)
		bool bIsPooled;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multicast_ReleasedFromPool();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multicast_ReturnedToPool();

	UPROPERTY()
		FTransform WhereWeWereBeforePooled;

	virtual bool IsWaterCraft_Implementation() const override;
	virtual bool IsPossessable_Implementation() const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC")
		TArray<AStructure*> StructuresAttached;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundBase* AnimalAttackSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundBase* SoundToPlayWhenPerceived;

	// 	UFUNCTION(BlueprintCallable, Category = "Networking")
	// 		void SetHibernate(bool bHibernate);

	// 	UFUNCTION()
	// 		void HibernationModeChanged();
	// 
	// 	UPROPERTY(ReplicatedUsing = OnRep_Hibernation)
	// 		bool bIsHibernating;
	// 
	// 	UFUNCTION()
	// 		void OnRep_Hibernation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
		float AttackSpeedTimer = 3.f;

	UFUNCTION()
		void PossessOriginalPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NPC")
		USphereComponent* SphereCollider;

	UFUNCTION(NetMulticast, Unreliable, WithValidation, Category = Animation)
		void Multicast_PlayTakeDamageAnimation();

	UPROPERTY(BlueprintReadWrite)
		bool LeftClickDown;

	UFUNCTION(Server, Unreliable, WithValidation)
		void Server_LeftClickDown();

	UFUNCTION(Server, Unreliable, WithValidation)
		void Server_LeftClickUp();

	UFUNCTION()
		void Dismount();

	UFUNCTION(Server, WithValidation, Reliable)
		void Server_Dismount();

	UPROPERTY(BlueprintReadWrite, Category = Mount)
		bool bMounted;

	UFUNCTION()
		void ZoomIn();

	UFUNCTION()
		void ZoomOut();

	UPROPERTY(BlueprintReadWrite, Category = Mount)
		float Stamina;

	UPROPERTY(BlueprintReadWrite, Category = Mount)
		class AVeritexCharacter* MountedCharacter;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mount)
		EMountName MountName;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Tribe)
		FString OwnedBy;

	UPROPERTY(BlueprintReadWrite, Category = Tribe)
		int32 TribeRideRank;

	UPROPERTY(BlueprintReadWrite, Category = Stats)
		int32 MovementSpeed;

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
		void Server_DismountAndMoveLeftOrRight();

	UFUNCTION()
		void TurnAtRate(float Rate);

	UFUNCTION()
		void JumpOrFly();

	UFUNCTION()
		bool CanMountFly();

	UFUNCTION()
		bool CanMountJump();

	UFUNCTION()
		void LookUpAtRate(float Rate);

	UFUNCTION()
		void MoveForward(float Value);

	UFUNCTION()
		void MoveRight(float Value);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Blood)
		UParticleSystem* P_BloodParticle;

	UPROPERTY()
		TArray<AVeritexCharacter*> PlayersWhoAttackedUs;

	UPROPERTY()
		bool bRemovedOneAIFromBiomeSpawner;

	UPROPERTY()
		bool bExperienceGivenFromDeath;

	UPROPERTY(BlueprintReadWrite, Replicated, EditAnywhere, Category = NPCs)
		FString AnimalName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		float ExperienceToGive;

	UPROPERTY(BlueprintReadWrite, Category = NPCs)
		bool bIsDead = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		UAnimMontage* UnconsciousMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		UAnimMontage* AttackMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		UAnimMontage* TakeDamageMontage;

	UPROPERTY()
		UPhysicalMaterial* BloodPhysMaterial;

	UPROPERTY(BlueprintReadWrite)
		class ABiomeSpawner* TheSpawnerThatSpawnedUs;

	UPROPERTY(EditAnywhere, Category = Behavior)
		class UBehaviorTree* BotBehavior;

	UPROPERTY()
		UNavigationInvokerComponent* NavigationInvoker;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mounts)
		float DismountAmount;

	UPROPERTY(EditDefaultsOnly, Category = NPCs)
		bool bCanBeMounted;

	UPROPERTY(EditDefaultsOnly, Category = NPCs)
		ENPCType NPCType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = NPCs)
		int32 Health;

	UPROPERTY(BlueprintReadWrite, Replicated, EditAnywhere, Category = NPCs)
		int32 MaxHealth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 Torpor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 MaxTorpor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 Food;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 MaxFood;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 Water;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		int32 MaxWater;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		float MaxMovementSpeed = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		float BaseDamage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs, meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
		float DamageResistanceAsPercent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = NPCs)
		int32 Level;

	UPROPERTY()
		float LevelModifier;

	UPROPERTY()
		UNavigationSystemV1* NavSystem;

	UPROPERTY()
		float BaseLookUpRate;

	UPROPERTY()
		float BaseTurnRate;

	UPROPERTY()
		int32 MaxLevel;

// 	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
// 		USpringArmComponent* CameraBoom;
// 
// 	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
// 		UCameraComponent* FollowCamera;

	UFUNCTION(Server, WithValidation, Unreliable)
		void Server_SetMovementMode(EMovementMode NewMode);

	// 	virtual bool IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer) override;
	// 
	// 	virtual void OnReplicationPausedChanged(bool bIsReplicationPaused) override;
	// 
	// 	void BuildPauseReplicationCheckPoints(TArray<FVector>& RelevancyCheckPoints);

	UFUNCTION(BlueprintImplementableEvent)
		void PlayDeathAnimationAndDestroy();

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void Multicast_PlayBloodParticle();

	UPROPERTY()
		bool bLootBagSpawned = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		float TorporDamageWhenHit = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = NPCs)
		float DamageToDealWhenHit = 0;

	UFUNCTION()
		void SetStats();

	void ReleasedFromPool(FTransform const& Transform);


	void ReturnedToPool();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<UAnimInstance> OriginalAnimInstance;

protected:

	virtual void BeginPlay() override;
	void UpdateRateParams(FAnimUpdateRateParameters* AnimUpdateRateParams);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

};
