// Daniel Gleason (C) 2017

#include "VeritexCharacter.h"
#include "Veritex.h"
#include "VeritexPlayerController.h"
#include "UnrealNetwork.h"
#include "VeritexFunctionLibrary.h"
#include "VeritexGameState.h"
#include "InstancedFoliageActor.h"
#include "VeritexCharacterMovement.h"

#include "Perception/AISense_Sight.h"
#include "NavigationInvokerComponent.h"
#include "UWorksCore/Public/Libraries/UWorksLibrarySteamID.h"
#include "UWorksCore/Public/Interfaces/UWorksInterfaceCoreUser.h"
#include "UWorksCore/Public/Interfaces/UWorksInterfaceCoreFriends.h"
#include "UWorksCore/Public/Libraries/UWorksLibraryCore.h"
#include "Interfaces/UWorksInterfaceCoreUserStats.h"

#include "Sound/SoundCue.h"
#include "VeritexAIController.h"

#include "VeritexStructs.h"

#include "NPC.h"

// If is first load, and MOVE_FALLING == true, lets spawn in a widget which covers the map until we are now in MOVE_WALKING, can use OnMovementModeChanged() for this.


//////////////////////////////////////////////////////////////////////////
// AVeritexCharacter
//////////////////////////////////////////////////////////////////////////

AVeritexCharacter::AVeritexCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UVeritexCharacterMovement>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.f; //450.f; // The camera follows at this distance behind the character	
	CameraBoom->RelativeLocation = FVector(0, 0, 10);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a CameraComponent	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->RelativeLocation = FVector(99.999992, 50.f, 50.0f);
	//FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FollowCamera->RelativeRotation = FRotator(0, -0.000031, 0);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 450.0f; //600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Called on Construct, if its loaded by the SQLite database the db will overwrite this.
	if (Role == ROLE_Authority)
	{
		Health = 100;
		MaxHealth = 100;
		Food = 100;
		MaxFood = 100;
		Water = 100;
		MaxWater = 100;
	}

	LowHealthPercentage = 0.5f;
}

// Temporary Client Sided Solution, should also check the server for time since last input. Should also do a check after this for the same thing but on the server. If it passes both the client and server sided tests, we can proceed.
bool AVeritexCharacter::CheckIfCanProceedWithInput()
{
	if (TimeSinceLastInput + 0.5f <= GetWorld()->GetTimeSeconds())
	{
		TimeSinceLastInput = GetWorld()->GetTimeSeconds();
		return true;
	}
	return false;
}

// End Input Delay

FRotator AVeritexCharacter::GetFootPrintRotation()
{
	FHitResult CurrentFloor = GetCharacterMovement()->CurrentFloor.HitResult;

	FRotator OutRotation = (-CurrentFloor.ImpactNormal).Rotation();
	OutRotation.Yaw = GetActorRotation().Yaw + 90.f;

	return OutRotation;
}


void AVeritexCharacter::TakeTorporDamage(float TorporAmount)
{
	Torpor += TorporAmount;
	// Could also create weapons which allow us to take less torpor damage.

	if (Torpor >= MaxTorporBeforeUnconscious)
	{
		//EnableDisableVingette(true);
	}
	else if (Torpor > MaxTorpor)
	{
		Torpor = MaxTorpor;
	}
}

void AVeritexCharacter::Client_InvokeBattleTheme_Implementation(bool inCombat)
{
	if (BattleTheme != nullptr)
	{
		if (BattleThemeComponent == nullptr)
		{
			BattleThemeComponent = UGameplayStatics::SpawnSoundAttached(BattleTheme, GetRootComponent(), NAME_None,
			                                                            FVector(ForceInit),
			                                                            EAttachLocation::KeepRelativeOffset, true);
			GetWorld()->GetTimerManager().SetTimer(BattleThemeTimerHandle, this, &AVeritexCharacter::StopBattleTheme,
			                                       30.f, false);
		}
		else
		{
			if (!BattleThemeComponent->IsPlaying() && inCombat)
			{
				BattleThemeComponent->Play();
				GetWorld()->GetTimerManager().SetTimer(BattleThemeTimerHandle, this,
				                                       &AVeritexCharacter::StopBattleTheme, 30.f, false);
			}
			if (!inCombat)
			{
				BattleThemeComponent->Stop();
			}
		}
	}
}

void AVeritexCharacter::StopBattleTheme()
{
	if (BattleThemeComponent != nullptr)
	{
		BattleThemeComponent->Stop();
	}
}

void AVeritexCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Super::FellOutOfWorld(dmgType);
}

bool AVeritexCharacter::IsInTribe()
{
	if (TribeRank > 0 || TribeID != "" || TribeName != "")
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
// REPLICATION
void AVeritexCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVeritexCharacter, UniqueSteamID);
	DOREPLIFETIME(AVeritexCharacter, TribeRank);
	DOREPLIFETIME(AVeritexCharacter, TribeID);
	DOREPLIFETIME(AVeritexCharacter, TribeName);
	DOREPLIFETIME(AVeritexCharacter, PlayerName);

	DOREPLIFETIME(AVeritexCharacter, Health);
	DOREPLIFETIME(AVeritexCharacter, MaxHealth);

	DOREPLIFETIME(AVeritexCharacter, Food);
	DOREPLIFETIME(AVeritexCharacter, MaxFood);

	DOREPLIFETIME(AVeritexCharacter, Water);
	DOREPLIFETIME(AVeritexCharacter, MaxWater);

	DOREPLIFETIME(AVeritexCharacter, Armor);

	DOREPLIFETIME(AVeritexCharacter, Oxygen);
	DOREPLIFETIME(AVeritexCharacter, Torpor);
	DOREPLIFETIME(AVeritexCharacter, MaxTorpor);

	DOREPLIFETIME(AVeritexCharacter, CurrentLevel);
	DOREPLIFETIME(AVeritexCharacter, Experience);

	DOREPLIFETIME_CONDITION(AVeritexCharacter, bWantsToRun, COND_SkipOwner);

	DOREPLIFETIME(AVeritexCharacter, IsLoggedIn);
	DOREPLIFETIME(AVeritexCharacter, InviteTribeID);
	DOREPLIFETIME(AVeritexCharacter, InviteTribeName);

	DOREPLIFETIME(AVeritexCharacter, IsCrawling);

	DOREPLIFETIME(AVeritexCharacter, Temperature);
	DOREPLIFETIME(AVeritexCharacter, TemperatureModifier);

	DOREPLIFETIME(AVeritexCharacter, Oxygen);
	DOREPLIFETIME(AVeritexCharacter, MaxOxygen);
	DOREPLIFETIME(AVeritexCharacter, MeleeDamage);
	DOREPLIFETIME(AVeritexCharacter, MovementSpeed);
	DOREPLIFETIME(AVeritexCharacter, MaxStamina);

	DOREPLIFETIME(AVeritexCharacter, LevelUpPoints);

	DOREPLIFETIME(AVeritexCharacter, bHideWeather);

}

void AVeritexCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	if (Role < ROLE_Authority)
	{
		Server_SetRunning(bNewRunning, bToggle);
	}
}

bool AVeritexCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return (bWantsToRun || bWantsToRunToggled) && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() |
		GetActorForwardVector()) > -0.1;
}

bool AVeritexCharacter::IsMoving()
{
	return FMath::Abs(GetLastMovementInputVector().Size()) > 0.f;
}

bool AVeritexCharacter::HasHealthRegen()
{
	return Food > 0 && Water > 0 && Oxygen > 0 && TemperatureAllowsHealthRegen ? true : false;
}

void AVeritexCharacter::Multicast_SpawnBloodFX_Implementation()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), P_BloodParticle, GetActorTransform());
}

bool AVeritexCharacter::Multicast_SpawnBloodFX_Validate()
{
	return true;
}

float AVeritexCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                    class AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Damage == 0)
	{
		return 0;
	}

	Damage -= Armor;

	Health -= Damage;

	Multicast_SpawnBloodFX();

	if (Health <= 0)
	{
		Health = 0;
		VPC->Client_OpenWorldMapWidget();
	}

	FlashHitEffect(DamageCauser);

	//ClientHideInventory();
	if (DamageCauser != this)
	{
		PlayHitSound();
		Client_InvokeBattleTheme(true);
	}

	AVeritexCharacter* VAC = Cast<AVeritexCharacter>(DamageCauser);
	if (VAC && DamageCauser != this)
	{
		VAC->Client_InvokeBattleTheme(true);
		PlayersWhoAttackedUs.Add(VAC);
	}

	return Damage;
}

void AVeritexCharacter::UnPossessed()
{
	Super::UnPossessed();
}

void AVeritexCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AVeritexPlayerController* PC = Cast<AVeritexPlayerController>(NewController);

	if (Controller != nullptr)
	{
		VPC = PC;
	}
}

void AVeritexCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (Role == ROLE_AutonomousProxy)
	{
		PlayLandingSound();
	}

	if (HasAuthority())
	{
		DistanceToGround_End = GetActorLocation();
		const float Distance = FMath::Max<float>(0.f, DistanceToGround_Start.Z - DistanceToGround_End.Z);
		const float Damage = FallingDamageCurve->GetFloatValue(Distance);

		FDamageEvent DamageEvent;
		TakeDamage(Damage, DamageEvent, GetController(), this);
	}
}

void AVeritexCharacter::Falling()
{
	Super::Falling();

	if (Role == ROLE_Authority)
	{
		DistanceToGround_Start = GetActorLocation();
	}
}

bool AVeritexCharacter::Server_SetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void AVeritexCharacter::Server_SetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	SetRunning(bNewRunning, bToggle);
}


//////////////////////////////////////////////////////////////////////////
// Input

void AVeritexCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVeritexCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVeritexCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Crawl", IE_Pressed, this, &AVeritexCharacter::Crawl);
}


void AVeritexCharacter::Crawl()
{
	if (IsCrawling)
	{
		Server_SetIsCrawling(false);
		UpdateLocomotionState(0);
	}
	else
	{
		Server_SetIsCrawling(true);
		UpdateLocomotionState(1);
	}
}

void AVeritexCharacter::Server_SetIsCrawling_Implementation(bool NewCrawlingState)
{
	IsCrawling = NewCrawlingState;
}

bool AVeritexCharacter::Server_SetIsCrawling_Validate(bool NewCrawlingState)
{
	return true;
}

void AVeritexCharacter::SetShiftKeyDown()
{
	SetRunning(true, false);
}

void AVeritexCharacter::SetShiftKeyUp()
{
	SetRunning(false, false);
}

void AVeritexCharacter::SteamAchievementsForGathering(FString ItemToAddName, int32 AmountToAdd)
{
	UUWorksInterfaceCoreUserStats* UserStats = UUWorksLibraryCore::GetUserStats();

	if (!UserStats)
	{
		return;
	}

	if (ItemToAddName.Equals("Stone"))
	{
		bool bCollected100Stone = false;
		UserStats->GetAchievement(FString("COLLECT_100_STONE"), bCollected100Stone);

		bool bCollected500Stone = false;
		UserStats->GetAchievement(FString("COLLECT_500_STONE"), bCollected500Stone);

		int32 STAT_StoneCollection;
		UserStats->GetStatAsInteger(FString("STONE_COLLECTION"), STAT_StoneCollection);
		STAT_StoneCollection += AmountToAdd;

		if (!bCollected100Stone)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_100_STONE"), STAT_StoneCollection, 100);
			UserStats->SetStatAsInteger(FString("STONE_COLLECTION"), STAT_StoneCollection);
			if (STAT_StoneCollection >= 100)
			{
				UserStats->SetAchievement(FString("COLLECT_100_STONE"));
			}
		}
		else if (!bCollected500Stone)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_500_STONE"), STAT_StoneCollection, 500);
			UserStats->SetStatAsInteger(FString("STONE_COLLECTION"), STAT_StoneCollection);
			if (STAT_StoneCollection >= 500)
			{
				UserStats->SetAchievement(FString("COLLECT_500_STONE"));
			}
		}
	}
	else if (ItemToAddName.Equals("WoodLogs"))
	{
		bool bCollected100Wood = false;
		UserStats->GetAchievement(FString("COLLECT_100_WOOD"), bCollected100Wood);

		bool bCollected500Wood = false;
		UserStats->GetAchievement(FString("COLLECT_500_WOOD"), bCollected500Wood);

		int32 STAT_WoodCollection;
		UserStats->GetStatAsInteger(FString("WOOD_COLLECTION"), STAT_WoodCollection);
		STAT_WoodCollection += AmountToAdd;

		if (!bCollected100Wood)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_100_WOOD"), STAT_WoodCollection, 100);
			UserStats->SetStatAsInteger(FString("WOOD_COLLECTION"), STAT_WoodCollection);
			if (STAT_WoodCollection >= 100)
			{
				UserStats->SetAchievement(FString("COLLECT_100_WOOD"));
			}
		}
		else if (!bCollected500Wood)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_500_WOOD"), STAT_WoodCollection, 500);
			UserStats->SetStatAsInteger(FString("WOOD_COLLECTION"), STAT_WoodCollection);
			if (STAT_WoodCollection >= 500)
			{
				UserStats->SetAchievement(FString("COLLECT_500_WOOD"));
			}
		}
	}
	else if (ItemToAddName.Equals("Thatch"))
	{
		bool bCollected100Thatch = false;
		UserStats->GetAchievement(FString("COLLECT_100_THATCH"), bCollected100Thatch);

		bool bCollected500Thatch = false;
		UserStats->GetAchievement(FString("COLLECT_500_THATCH"), bCollected500Thatch);

		int32 STAT_ThatchCollection;
		UserStats->GetStatAsInteger(FString("THATCH_COLLECTION"), STAT_ThatchCollection);
		STAT_ThatchCollection += AmountToAdd;

		if (!bCollected100Thatch)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_100_THATCH"), STAT_ThatchCollection, 100);
			UserStats->SetStatAsInteger(FString("THATCH_COLLECTION"), STAT_ThatchCollection);
			if (STAT_ThatchCollection >= 100)
			{
				UserStats->SetAchievement(FString("COLLECT_100_THATCH"));
			}
		}
		else if (!bCollected500Thatch)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_500_WOOD"), STAT_ThatchCollection, 500);
			UserStats->SetStatAsInteger(FString("WOOD_COLLECTION"), STAT_ThatchCollection);
			if (STAT_ThatchCollection >= 500)
			{
				UserStats->SetAchievement(FString("COLLECT_500_THATCH"));
			}
		}
	}
	else if (ItemToAddName.Equals("Hemp"))
	{
		bool bCollected20Hemp = false;
		UserStats->GetAchievement(FString("COLLECT_20_HEMP"), bCollected20Hemp);

		int32 STAT_HempCollection;
		UserStats->GetStatAsInteger(FString("HEMP_COLLECTION"), STAT_HempCollection);
		STAT_HempCollection += AmountToAdd;

		if (!bCollected20Hemp)
		{
			UserStats->IndicateAchievementProgress(FString("COLLECT_20_HEMP"), STAT_HempCollection, 100);
			UserStats->SetStatAsInteger(FString("HEMP_COLLECTION"), STAT_HempCollection);
			if (STAT_HempCollection >= 100)
			{
				UserStats->SetAchievement(FString("COLLECT_20_HEMP"));
			}
		}
	}
	UserStats->StoreStats();
}

void AVeritexCharacter::SteamAchievementsForCrafting(FString ItemCrafted)
{
	UUWorksInterfaceCoreUserStats* UserStats = UUWorksLibraryCore::GetUserStats();

	if (!UserStats)
	{
		return;
	}

	if (ItemCrafted.Contains("Wall") || ItemCrafted.Contains("Foundation") || ItemCrafted.Contains("Ceiling") ||
		ItemCrafted.Contains("Door"))
	{
// 		bool bCreatedBaseComponent = false;
// 		UserStats->GetAchievement("CREATE_BASE_COMPONENT", bCreatedBaseComponent);
// 		if (!bCreatedBaseComponent)
// 		{
			UserStats->SetAchievement(FString("CREATE_BASE_COMPONENT"));
//		}
	}
	if (!ItemCrafted.Equals("Hemp") && ItemCrafted.Contains("Hemp"))
	{
// 		bool bCraftedClothing = false;
// 		UserStats->GetAchievement("CRAFT_CLOTHING", bCraftedClothing);
// 		if (!bCraftedClothing)
// 		{
			UserStats->SetAchievement(FString("CRAFT_CLOTHING"));
//		}
	}
	if (ItemCrafted.Contains("Furnace"))
	{
// 		bool bCraftedFurnace = false;
// 		UserStats->GetAchievement(FString("CRAFT_FURNACE"), bCraftedFurnace);
// 		if (!bCraftedFurnace)
// 		{
			UserStats->SetAchievement(FString("CRAFT_FURNACE"));
//		}
	}
	if (ItemCrafted.Equals("CampFire") || ItemCrafted.Contains("Camp"))
	{
		//bool bCraftedCampfire = false;
		//UserStats->GetAchievement(FString("CRAFT_CAMPFIRE"), bCraftedCampfire);
		//if (!bCraftedCampfire)
		//{
			UserStats->SetAchievement(FString("CRAFT_CAMPFIRE"));
		//}
	}
	if (ItemCrafted.Equals("Bed"))
	{
		//bool bCraftedBed = false;
		//UserStats->GetAchievement(FString("CRAFT_BED"), bCraftedBed);
		//if (!bCraftedBed)
		//{
			UserStats->SetAchievement(FString("CRAFT_BED"));
		//}
	}

	UserStats->StoreStats();
}

void AVeritexCharacter::SteamAchievementsForPlacingStructures(FString StructurePlaced)
{
	UUWorksInterfaceCoreUserStats* UserStats = UUWorksLibraryCore::GetUserStats();

	if (!UserStats)
	{
		return;
	}

	if (StructurePlaced.Equals("CampFire") || StructurePlaced.Contains("Fire"))
	{
		//bool bPlacedCampFire = false;
		//UserStats->GetAchievement("PLACE_CAMPFIRE", bPlacedCampFire);
		//if (!bPlacedCampFire)
		//{
			UserStats->SetAchievement(FString("PLACE_CAMPFIRE"));
		//}
	}
	else if (StructurePlaced.Equals("Bed"))
	{
		//bool bPlacedBed = false;
		//UserStats->GetAchievement("PLACE_BED", bPlacedBed);
		//if (!bPlacedBed)
		//{
			UserStats->SetAchievement(FString("PLACE_BED"));
		//}
	}

	UserStats->StoreStats();
}

void AVeritexCharacter::WeatherCheck()
{
	AVeritexGameState* VGS = Cast<AVeritexGameState>(GetWorld()->GetGameState());
	if (VGS)
	{
		FHitResult OutHit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(),
		                                         GetActorUpVector() * 1000 + GetActorLocation(), ECC_Visibility,
		                                         Params))
		{
			UpdateWeather(EWeatherMode::ClearSkys);
			bHideWeather = true;
			return;
		}

		bHideWeather = false;
		UpdateWeather(VGS->WeatherMode);
	}
}

void AVeritexCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		FTimerHandle WeatherCheck;
		GetWorld()->GetTimerManager().SetTimer(WeatherCheck, this, &AVeritexCharacter::WeatherCheck, .5f, true);

		FTimerHandle TemperatureCheck;
		GetWorld()->GetTimerManager().SetTimer(TemperatureCheck, this, &AVeritexCharacter::CheckTemperature, 10.f,
		                                       true);

		if (CurrentLevel >= MaxLevel)
		{
			return;
		}

		FTimerHandle PassiveExperience;
		GetWorld()->GetTimerManager().SetTimer(PassiveExperience, this, &AVeritexCharacter::PassiveExperience, 1, true,
		                                       5.0);

		PlayerName = GetPersonaName();
	}
}

void AVeritexCharacter::CheckTemperature()
{
	Temperature = FMath::RandRange(30, 99);
	// If no volumes are telling us, pick a random range between 50 and 70 degrees.

	FHitResult OutHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), GetActorLocation() + (GetActorUpVector() * 5000),
	                                     ECC_Visibility, Params);

	AStructure* StructureHit = Cast<AStructure>(OutHit.GetActor());

	if (StructureHit)
	{
		if (StructureHit->StructureType == EStructureType::Ceiling)
		{
			if (Temperature >= 100)
			{
				Temperature -= InsideHouseInsulationAmount;
			}
			if (Temperature <= 30)
			{
				Temperature += InsideHouseInsulationAmount;
			}
		}
	}

	if (Temperature >= 100)
	{
		TemperatureAllowsHealthRegen = false;
		Health -= HealthHeatedDecrementAmount;
	}
	else if (Temperature <= 30)
	{
		TemperatureAllowsHealthRegen = false;
		Health -= HealthFreezingDecrementAmount;
	}
	else
	{
		TemperatureAllowsHealthRegen = true;
	}

	if (Temperature < 45)
	{
		Food -= FoodFreezingAmount;
	}

	if (Temperature > 90)
	{
		Water -= WaterOverHeatedAmount;
	}

	if (GetCharacterMovement()->IsSwimming())
	{
		Temperature -= SwimmingTemperatureModifier;
	}


	UpdateHUDTemperatureIcons_Implementable(Temperature);
}


void AVeritexCharacter::PassiveExperience()
{
	ReceiveExperience(0.05);
}

void AVeritexCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (AVeritexCharacter* Char : PlayersWhoAttackedUs)
	{
		if (Char != nullptr)
		{
			Char->Client_InvokeBattleTheme(false);
		}
	}
}

void AVeritexCharacter::Server_DealDamage_Implementation(int healthAmount)
{
	Health -= healthAmount;
}

bool AVeritexCharacter::Server_DealDamage_Validate(int healthAmount)
{
	return true;
}

void AVeritexCharacter::Multicast_UseDoor_Implementation(AStructure* TheDoor, bool bOpen)
{
	if (TheDoor)
	{
		FRotator currentRotation = TheDoor->GetActorRotation();

		if (bOpen)
		{
			FRotator newRotation = FRotator(currentRotation.Pitch, currentRotation.Yaw - 90, currentRotation.Roll);
			TheDoor->SetActorRotation(newRotation);
		}
		else
		{
			FRotator newRotation = FRotator(currentRotation.Pitch, currentRotation.Yaw + 90, currentRotation.Roll);
			TheDoor->SetActorRotation(newRotation);
		}

		TheDoor->bIsDoorOpen = bOpen;
	}
}

bool AVeritexCharacter::Multicast_UseDoor_Validate(AStructure* TheDoor, bool bOpen)
{
	return true;
}

void AVeritexCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		if (Torpor <= 10)
		{
			//EnableDisableVingette(false);
		}
	}

	// 	if (HasAuthority())
	// 	{
	// 		// Need to keep a check on this to make sure it is not dangerous or using up too much resources.
	// 		if (!IsPendingKill())
	// 		{
	// 			if (Health <= 0)
	// 			{
	// 				Destroy();
	// 			}
	// 		}
	// 	}


	if (IsLocallyControlled())
	{
		if (LowHealthSound)
		{
			if ((this->Health > 0 && this->Health < this->MaxHealth * LowHealthPercentage) && (!LowHealthWarningPlayer
				|| !LowHealthWarningPlayer->IsPlaying()))
			{
				LowHealthWarningPlayer = UGameplayStatics::SpawnSoundAttached(LowHealthSound, GetRootComponent(),
				                                                              NAME_None, FVector(ForceInit),
				                                                              EAttachLocation::KeepRelativeOffset,
				                                                              true);
				if (LowHealthWarningPlayer != nullptr)
				{
					LowHealthWarningPlayer->SetVolumeMultiplier(0.0f);
					//EnableDisableDamageVingette(false);
				}
			}
			else if ((this->Health > this->MaxHealth * LowHealthPercentage || this->Health < 0) &&
				LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
			{
				LowHealthWarningPlayer->Stop();
				//EnableDisableDamageVingette(false);
			}
			if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
			{
				const float MinVolume = 0.3f;
				const float VolumeMultiplier = (1.0f - (this->Health / (this->MaxHealth * LowHealthPercentage)));
				LowHealthWarningPlayer->SetVolumeMultiplier(MinVolume + (1.0f - MinVolume) * VolumeMultiplier);
				//EnableDisableDamageVingette(true);
			}
		}


		UpdateRunSounds();
	}
}

void AVeritexCharacter::UpdateRunSounds()
{
	const bool bIsRunSoundPlaying = RunLoopAC != nullptr && RunLoopAC->IsActive();
	const bool bWantsRunSoundPlaying = IsRunning() && IsMoving() && !bIsCrouched && !bIsMovingLeftOrRight && !
		GetCharacterMovement()->IsFalling();

	// Don't bother playing the sounds unless we're running and moving.
	if (!bIsRunSoundPlaying && bWantsRunSoundPlaying)
	{
		if (RunLoopAC != nullptr)
		{
			RunLoopAC->Play();
		}
		else if (RunLoopSound != nullptr)
		{
			RunLoopAC = UGameplayStatics::SpawnSoundAttached(RunLoopSound, GetRootComponent());
			if (RunLoopAC != nullptr)
			{
				RunLoopAC->bAutoDestroy = false;
			}
		}
	}
	else if (bIsRunSoundPlaying && !bWantsRunSoundPlaying)
	{
		RunLoopAC->Stop();
		if (RunStopSound != nullptr)
		{
			UGameplayStatics::SpawnSoundAttached(RunStopSound, GetRootComponent());
		}
	}
}

float AVeritexCharacter::GetExperienceRequiredForNextLevel()
{
	float exp = 0;
	for (int i = 1; i < CurrentLevel + 1; i++)
	{
		exp += FMath::FloorToInt(i + 300 * FMath::Pow(2, i / 7.0));
	}

	float expOut = FMath::FloorToInt(exp / 4);
	return expOut;
}

float AVeritexCharacter::GetExperienceRequiredForNextLevelBasedOnCurrentLevel(int32 InCurrentLevel)
{
	float exp = 0;
	for (int i = 1; i < InCurrentLevel + 1; i++)
	{
		exp += FMath::FloorToInt(i + 300 * FMath::Pow(2, i / 7.0));
	}

	float expOut = FMath::FloorToInt(exp / 4);
	return expOut;
}


void AVeritexCharacter::ReceiveExperience(float Amount)
{
	bool bLeveledUp = Experience + Amount >= GetExperienceRequiredForNextLevel() ? true : false;

	if (bLeveledUp)
	{
		if (CurrentLevel + 1 >= MaxLevel)
		{
			DisplayMessage(FText::FromString("You have reached the max level!"));

			UUWorksInterfaceCoreUserStats* UserStats = UUWorksLibraryCore::GetUserStats();

			if (!UserStats)
			{
				return;
			}
			UserStats->SetAchievement(FString("MAX_LEVEL"));

			return;
		}

		CurrentLevel++;
		LevelUpPoints++;
		InvokeLevelUp(
			FText::FromString(FString("You have leveled up to level ") + FString::FromInt(CurrentLevel) + "!"));
		Client_PlayLevelUpSound();
	}

	Experience += Amount;
	SetNewLevelAndUpdateExperienceBar(FText::FromString(FString::FromInt(CurrentLevel)),
	                                  FText::FromString(FString::SanitizeFloat(Experience)));
}

void AVeritexCharacter::Client_PlayLevelUpSound_Implementation()
{
	if (LevelUpAudioComponent)
	{
		if (!LevelUpAudioComponent->IsPlaying())
		{
			LevelUpAudioComponent->Play();
		}
	}
	else
	{
		LevelUpAudioComponent = UGameplayStatics::SpawnSoundAttached(LevelUpSoundCue, GetRootComponent(), NAME_None,
		                                                             FVector(ForceInit),
		                                                             EAttachLocation::KeepRelativeOffset, true);
	}
}

FRotator AVeritexCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool AVeritexCharacter::IsThisCharacterPendingKill()
{
	return this->IsPendingKill();
}

FString AVeritexCharacter::GetSteamID()
{
	if (UUWorksLibraryCore::GetUser())
	{
		return UUWorksLibrarySteamID::GetIdentifier(UUWorksLibraryCore::GetUser()->GetSteamID());
	}
	return FString("0");
}

FString AVeritexCharacter::GetPersonaName()
{
	if (UUWorksLibraryCore::GetFriends())
	{
		return UUWorksLibraryCore::GetFriends()->GetPersonaName();
	}
	return FString("");
}

void AVeritexCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AVeritexCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AVeritexCharacter::MoveForward_Implementation(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)) //  && Health > 0
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AVeritexCharacter::MoveRight_Implementation(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)) //  && Health > 0
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

UAudioComponent* AVeritexCharacter::CreateVoiceAudioComponent()
{
	UAudioComponent* AudioComponent = nullptr;

	if (GEngine != nullptr)
	{
		FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			USoundWaveProcedural* SoundStreaming = NewObject<USoundWaveProcedural>();
			SoundStreaming->SetSampleRate(48000);
			SoundStreaming->NumChannels = 1;
			SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
			SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
			SoundStreaming->bLooping = false;

			FAudioDevice::FCreateComponentParams Params(GetWorld());
			AudioComponent = FAudioDevice::CreateComponent(SoundStreaming, Params);

			if (AudioComponent)
			{
				AudioComponent->AttachToComponent(RootComponent,
				                                  FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				AudioComponent->bIsUISound = true;
				AudioComponent->bAllowSpatialization = true;
				AudioComponent->SetVolumeMultiplier(1.5f);
				AudioComponent->AttenuationSettings = VoIPAttentuation;


				const FStringAssetReference VoiPSoundClassName = GetDefault<UAudioSettings>()->VoiPSoundClass;
				if (VoiPSoundClassName.IsValid())
				{
					AudioComponent->SoundClassOverride = LoadObject<USoundClass>(
						nullptr, *VoiPSoundClassName.ToString());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Unable to create voice audio component!"));
			}
		}
	}

	return AudioComponent;
}

void AVeritexCharacter::PlayRawBufferOnAudioComponent(TArray<uint8> VoiceBuffer)
{
	if (!VOIPComponent)
	{
		VOIPComponent = CreateVoiceAudioComponent();
		if (VOIPComponent)
		{
			VOIPComponent->Play();
		}
	}

	if (!VOIPComponent)
	{
		GLog->Log("VoIP component not valid!");
		return;
	}

	USoundWaveProcedural* SoundStreaming = CastChecked<USoundWaveProcedural>(VOIPComponent->Sound);
	GLog->Log("About to queue sound!");

	if (SoundStreaming->GetAvailableAudioByteCount() == 0)
	{
		GLog->Log("VOIP audio component was starved!");
	}
	else
	{
		GLog->Log("Available byte count is: " + FString::FromInt(SoundStreaming->GetAvailableAudioByteCount()));
	}
	if (VoiceBuffer.Num() == 0)
	{
		GLog->Log("No voice buffer to play");
	}
	GLog->Log("Available byte count is: " + FString::FromInt(SoundStreaming->GetAvailableAudioByteCount()));

	SoundStreaming->QueueAudio(VoiceBuffer.GetData(), VoiceBuffer.Num());
}