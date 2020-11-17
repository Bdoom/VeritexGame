// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/Pawn.h"
#include "GameFramework//Character.h"
#include "NPC.h"
#include "BaseMountPawn.generated.h"

UCLASS()
class VERITEX_API ABaseMountPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseMountPawn(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	class UCameraComponent* FollowCamera;

	class USpringArmComponent* CameraBoom;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void LookUpAtRate(float Rate);

	void TurnAtRate(float Rate);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mount)
		class AVeritexCharacter* MountedCharacter;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Tribe)
		FString OwnedByTribeID;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Tribe)
		int32 TribeRideRank;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Stats)
		int32 Health;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Stats)
		int32 MovementSpeed;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Stats)
		int32 Stamina;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mount)
		EMountName MountName;

	UFUNCTION(BlueprintCallable, Category = Mount)
		void JumpOrFly();

	UFUNCTION(BlueprintPure, Category = Mounts)
		bool CanMountFly();

	UFUNCTION(BlueprintPure, Category = Mounts)
		bool CanMountJump();

	UFUNCTION(Server, WithValidation, Unreliable, Category = Mounts)
		void Server_SetMovementMode(EMovementMode NewMode);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Unreliable, Category = Mounts)
		void Server_DismountAndMoveLeftOrRight();

	UPROPERTY(BlueprintReadWrite, Category = Mounts)
		float DismountAmount;

};
