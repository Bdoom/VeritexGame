// Daniel Gleason (C) 2017

#pragma once
#include "GameFramework/Character.h"
#include "EngineMinimal.h"
#include "VeritexFoliageInstance.h"
#include "VoxelCharacter.h"
#include "VeritexStructs.h"

class UAudioComponent;
class USoundCue;

#include "VeritexCharacter.generated.h"

UCLASS(config = Game)
class AVeritexCharacter : public AVoxelCharacter
{
	GENERATED_BODY()

public:

	AVeritexCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Weather)
	bool bHideWeather;

	UPROPERTY(BlueprintReadWrite, Category = Weather)
	EWeatherMode OurWeatherMode;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeather(EWeatherMode NewWeatherMode);

	// Temporary Client Sided Solution, should also check the server for time since last input.
	UFUNCTION(BlueprintCallable, Category = "Input Delay")
	bool CheckIfCanProceedWithInput();

	UPROPERTY()
	float TimeSinceLastInput;

	// End Input Delay

	UFUNCTION(BlueprintPure, Category = "Foot Prints")
	FRotator GetFootPrintRotation();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Fall Damage")
	class UCurveFloat* FallingDamageCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Combat)
	UParticleSystem* P_BloodParticle;

	UFUNCTION(NetMulticast, WithValidation, Unreliable, BlueprintCallable, Category = "Particles")
	void Multicast_SpawnBloodFX();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UFUNCTION(BlueprintImplementableEvent)
	void EnableDisableDamageVingette(bool NewEnable);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	int32 LevelUpPoints = 0;

	UFUNCTION(BlueprintImplementableEvent, Category = Combat)
	void EnableDisableVingette(bool EnableOrDisable);

	UFUNCTION(BlueprintCallable, Category = Damage)
	void TakeTorporDamage(float TorporAmount);

	UPROPERTY()
	float MaxTorporBeforeUnconscious = 80.f;

	UPROPERTY()
	float SwimmingTemperatureModifier = 20.f;

	UPROPERTY()
	float InsideHouseInsulationAmount = 30;

	UPROPERTY()
	bool TemperatureAllowsHealthRegen;

	UPROPERTY()
	float WaterOverHeatedAmount = 3.f;

	UPROPERTY()
	float FoodFreezingAmount = 3.f;

	UPROPERTY()
	float HealthHeatedDecrementAmount = 1.f;

	UPROPERTY()
	float HealthFreezingDecrementAmount = 1.f;

	UPROPERTY(EditAnywhere, Category = PlayerSound)
	USoundCue* LevelUpSoundCue;

	UPROPERTY()
	UAudioComponent* LevelUpAudioComponent;

	UFUNCTION(Client, Unreliable)
	void Client_PlayLevelUpSound();

	UFUNCTION()
	void CheckTemperature();

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere)
	float Temperature;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Temperature)
	float TemperatureModifier;

	UFUNCTION(BlueprintImplementableEvent, Category = Temperature)
	void UpdateHUDTemperatureIcons_Implementable(float NewTemperature);

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool IsCrawling;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMovingLeftOrRight;

	UFUNCTION()
	void Crawl();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateLocomotionState(int32 StateNumber);

	UFUNCTION(BlueprintCallable, Server, Unreliable, WithValidation)
	void Server_SetIsCrawling(bool NewCrawlingState);

	UFUNCTION(BlueprintCallable, Client, Unreliable, Category = Combat)
	void Client_InvokeBattleTheme(bool inCombat);

	UPROPERTY()
	TArray<AVeritexCharacter*> PlayersWhoAttackedUs;

	UPROPERTY()
	FTimerHandle BattleThemeTimerHandle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Combat)
	USoundCue* BattleTheme;

	UPROPERTY()
	UAudioComponent* BattleThemeComponent;

	UFUNCTION(BlueprintCallable, Category = Combat)
	void StopBattleTheme();

	UFUNCTION()
	void PassiveExperience();

	UFUNCTION(BlueprintCallable, NetMulticast, WithValidation, Reliable, Category = Structures)
	void Multicast_UseDoor(AStructure* TheDoor, bool bOpen);

	UPROPERTY()
	FVector DistanceToGround_Start;

	UPROPERTY()
	FVector DistanceToGround_End;

	UFUNCTION()
	void UpdateRunSounds();

	UPROPERTY()
	float LowHealthPercentage;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayHitSound();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayLandingSound();

	UFUNCTION(BlueprintImplementableEvent)
	void ClientHideInventory();

	UFUNCTION(BlueprintImplementableEvent)
	void FlashHitEffect(AActor* AttackingActor);

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayMessage(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void SetNewLevelAndUpdateExperienceBar(const FText& NewLevel, const FText& NewExperience);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Player)
	int32 CurrentLevel = 1;

	UPROPERTY()
	int32 MaxLevel = 99;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Player)
	float Experience;

	UFUNCTION(BlueprintPure, Category = Player)
	float GetExperienceRequiredForNextLevel();

	UFUNCTION(BlueprintPure, Category = Player)
	float GetExperienceRequiredForNextLevelBasedOnCurrentLevel(int32 InCurrentLevel);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Player)
	float Armor = 0;

	UFUNCTION(BlueprintCallable, Category = Player)
	void ReceiveExperience(float Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = Player)
	void InvokeLevelUp(const FText& LevelUpMessage);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = Player)
	float Torpor = 0;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Player)
	float MaxTorpor = 100;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* LowHealthSound;

	/** sound played when running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunLoopSound;

	/** sound played when stop running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunStopSound;

	/** sound played on death, local player only */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Pawn)
	USoundCue* DeathSound;

	/** used to manipulate with run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USpringArmComponent* CameraBoom;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	UFUNCTION(BlueprintPure)
	bool IsThisCharacterPendingKill();

	UFUNCTION(BlueprintPure, Category = Steam)
	FString GetSteamID();
	UFUNCTION(BlueprintPure, Category = Steam)
	FString GetPersonaName();

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Tribe)
	FString InviteTribeID; // The ID of the persons tribe who is inviting you to their tribe.

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Tribe)
	FString InviteTribeName;

	void FellOutOfWorld(const class UDamageType& dmgType) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Online)
	bool IsLoggedIn;

	UFUNCTION(BlueprintPure, Category = Tribe)
	bool IsInTribe();

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Tribe)
	int TribeRank;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Tribe)
	FString TribeID;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Tribe)
	FString PlayerName;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Tribe)
	FString UniqueSteamID;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere, Category = Tribe)
	FString TribeName;

	UPROPERTY(BlueprintReadWrite, Category = Structures)
	FTransform LocalStructureTransform;

	UPROPERTY(BlueprintReadWrite, Category = Structures)
	bool IsInPlaceMode;

	UPROPERTY(BlueprintReadWrite, Category = Player)
	class AVeritexPlayerController* VPC;

	UFUNCTION(BlueprintImplementableEvent)
	void KillPlayerOffline();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(BlueprintReadWrite, Category = Camera)
	float CameraZoomSpeed = 100.f;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Structures)
	FTransform StructureTransform;

	UFUNCTION()
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Replicated)
	uint8 bWantsToRun : 1;

	UPROPERTY()
	uint8 bWantsToRunToggled : 1;

	UFUNCTION()
	void SetRunning(bool bNewRunning, bool bToggle);

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_SetRunning(bool bNewRunning, bool bToggle);

	UFUNCTION(BlueprintPure)
	bool IsRunning() const;

	UFUNCTION(BlueprintPure)
	bool IsMoving();

	UFUNCTION(BlueprintPure)
	bool HasHealthRegen();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerInfo)
	float MaxOxygen = 100;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerInfo)
	float MeleeDamage = 10;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerInfo)
	float MovementSpeed = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerInfo)
	float MaxStamina = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float Health = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float MaxHealth = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float Food = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float MaxFood = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float Water = 100;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = PlayerInfo)
	float MaxWater = 100;

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = Damage)
	void Server_DealDamage(int healthAmount);

	UPROPERTY(BlueprintReadWrite, Category = PlayerInfo)
	FGuid UID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = VOIP)
	USoundAttenuation* VoIPAttentuation;

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	                 AActor* DamageCauser) override;

	void UnPossessed() override;

	void PossessedBy(AController* NewController) override;

	void Landed(const FHitResult& Hit) override;

	void Falling() override;

	UFUNCTION(BlueprintCallable)
	void SetShiftKeyDown();

	UFUNCTION(BlueprintCallable)
	void SetShiftKeyUp();

	UPROPERTY(Replicated, BlueprintReadWrite)
	float Oxygen = 100;

	UFUNCTION(BlueprintCallable, Category = "Steam Achivements")
	void SteamAchievementsForGathering(FString ItemToAddName, int32 AmountToAdd);

	UFUNCTION(BlueprintCallable, Category = "Steam Achivements")
	void SteamAchievementsForCrafting(FString ItemCrafted);

	UFUNCTION(BlueprintCallable, Category = "Steam Achivements")
	void SteamAchievementsForPlacingStructures(FString StructurePlaced);

	UFUNCTION()
	void WeatherCheck();

protected:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
	void MoveForward(float Value);

	/** Called for side to side input */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Steam Voice")
	void PlayRawBufferOnAudioComponent(TArray<uint8> VoiceBuffer);

	UFUNCTION()
	UAudioComponent* CreateVoiceAudioComponent();

	UPROPERTY()
	UAudioComponent* VOIPComponent;

protected:
	// APawn interface
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void BeginPlay() override;

	void Tick(float DeltaSeconds) override;

public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
