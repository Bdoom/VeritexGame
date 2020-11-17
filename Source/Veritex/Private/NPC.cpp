// Daniel Gleason (C) 2017

#include "NPC.h"
#include "Veritex.h"
#include "NavigationSystem.h"
#include "NavigationInvokerComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VeritexAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "VeritexCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "BaseMountPawnMovement.h"
#include "BiomeSpawner.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "VeritexPlayerController.h"

static int32 NetEnablePauseRelevancy = 1;
FAutoConsoleVariableRef CVarNetEnablePauseRelevancy(
	TEXT("p.NetEnablePauseRelevancy"),
	NetEnablePauseRelevancy,
	TEXT("")
	TEXT("0: Disable, 1: Enable"),
	ECVF_Cheat);

// https://docs.unrealengine.com/en-us/Engine/Animation/Optimization

ANPC::ANPC(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMountPawnMovement>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NavigationInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("Navigation Invoker"));
	bReplicates = true;
	bReplicateMovement = true;

	AIControllerClass = AVeritexAIController::StaticClass();
	//AutoPossessAI = EAutoPossessAI::Disabled;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> Blood(
		TEXT("PhysicalMaterial'/Game/Impacts/Materials/Physical_Materials/Blood.Blood'"));
	BloodPhysMaterial = Blood.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> P_Blood(
		TEXT("ParticleSystem'/Game/FXVillesBloodVFXPack/Particles/PS_Blood_Spurt_01.PS_Blood_Spurt_01'"));
	P_BloodParticle = P_Blood.Object;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetMesh()->bEnableUpdateRateOptimizations = true;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 450.0f; //600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	//GetCharacterMovement()->bSweepWhileNavWalking = false; // not sure if we can change movement mode to NAV_WALKING for navmesh walking to improve performance... or maybe wait until 4.19 updates
	GetCharacterMovement()->MaxFlySpeed = 0;
	
	MaxLevel = 50;
	//GetCharacterMovement()->bAlwaysCheckFloor = false; // Not sure if this is dangerous or not, testing for performance issues.

// 	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
// 	CameraBoom->SetupAttachment(RootComponent);
// 	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

// 	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
// 	FollowCamera->SetupAttachment(CameraBoom);
// 	FollowCamera->bUsePawnControlRotation = false;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Attack Range Collider"));
	SphereCollider->InitSphereRadius(300.f);
	SphereCollider->SetupAttachment(GetMesh());

	GetMesh()->OnAnimUpdateRateParamsCreated.BindUObject(this, &ANPC::UpdateRateParams);

	//GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	//GetMesh()->SetCollisionObjectType(NPCObjectChannel);
	//GetMesh()->SetCollisionResponseToChannel(AICheckerObjectChannel, ECollisionResponse::ECR_Block);
	//GetMesh()->bGenerateOverlapEvents = true;

	//GetMesh()->bGenerateOverlapEvents = false;
	//GetCapsuleComponent()->bGenerateOverlapEvents = false;

	//  	GetCharacterMovement()->DefaultLandMovementMode = EMovementMode::MOVE_NavWalking;
	//  	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_NavWalking;
	//  	GetCharacterMovement()->SetGroundMovementMode(EMovementMode::MOVE_NavWalking);


	GetCharacterMovement()->MovementMode = MOVE_Walking;
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Walking;
	GetCharacterMovement()->SetGroundMovementMode(MOVE_Walking);
	//GetCharacterMovement()->bRunPhysicsWithNoController = false;

	//GetMesh()->bEnableUpdateRateOptimizations = true;
	//GetMesh()->bComponentUseFixedSkelBounds = true;

	//GetCharacterMovement()->bAlwaysCheckFloor = false;

	//NetDormancy = ENetDormancy::DORM_Initial;
	//NetUpdateFrequency = 66;
	//NetCullDistanceSquared /= 4;

	//NetUpdateFrequency = 66.0f;
	//MinNetUpdateFrequency = 2.f;
	//NetCullDistanceSquared = 1000000.0;

	//MinNetUpdateFrequency = 33.0f;

	//AAIController::RunBehaviorTree()
	//BrainComponent::PauseTree/Logic()
	//BrainComponent::StopTree();

	//GetCharacterMovement()->bUpdateOnlyIfRendered = true;

	//PlayerChecker = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerChecker"));
	//PlayerChecker->InitSphereRadius(20000.f);
	//PlayerChecker->OnComponentBeginOverlap.AddDynamic(this, &ANPC::PlayerCheckerBeginOverlap);
	//PlayerChecker->OnComponentEndOverlap.AddDynamic(this, &ANPC::PlayerCheckerEndOverlap);

	//bIsHibernating = false;
}

bool ANPC::IsWaterCraft_Implementation() const
{
	if (MountName == EMountName::Raft)
	{
		return true;
	}
	return false;
}

bool ANPC::IsPossessable_Implementation() const
{
	if (MountName == EMountName::Raft)
	{
		return true;
	}
	return false;
}

// void ANPC::SetHibernate(bool bHibernate)
// {
// 	if (!HasAuthority())
// 	{
// 		return;
// 	}
// 
// 	if (bIsHibernating != bHibernate)
// 	{
// 		bIsHibernating = bHibernate;
// 		HibernationModeChanged();
// 	}
// }
// 
// void ANPC::HibernationModeChanged()
// {
// 	if (bIsHibernating)
// 	{
// 		//NetUpdateFrequency = 0.01;
// 		//NetPriority = 0.01f;
// 
// 		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
// 		GetCapsuleComponent()->DestroyPhysicsState();
// 		//SetNetDormancy(DORM_DormantAll);
// 
// 		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
// 		GetMesh()->DestroyPhysicsState();
// 		GetMesh()->SetVisibility(false);
// 
// 		//if (HasAuthority())
// 		//{
// 			//SetReplicates(false);
// 			//SetReplicateMovement(false);
// 		//}
// 
// 		RegisterAllActorTickFunctions(false, true);
// 
// 		//GetCharacterMovement()->SetComponentTickEnabled(false);
// 
// 		// Disable Navigation Invoker
// 		//UNavigationSystemV1::UnregisterNavigationInvoker(*this);
// 
// 	}
// 	else
// 	{
// 		//SetNetDormancy(DORM_Awake);
// 		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
// 		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
// 		GetMesh()->SetVisibility(true);
// 		
// 		//NetUpdateFrequency = GetDefault<ACharacter>(GetClass())->NetUpdateFrequency;
// 		//NetPriority = GetDefault<ACharacter>(GetClass())->NetPriority;
// 
// 		if (HasAuthority())
// 		{
// 			//SetReplicates(true);
// 			//SetReplicateMovement(true);
// 		}
// 
// 		RegisterAllActorTickFunctions(true, true);
// 		//GetCharacterMovement()->SetComponentTickEnabled(true);
// 
// 		// Disable Navigation Invoker
// 		//UNavigationSystemV1::RegisterNavigationInvoker(*this, 500.f, 1500.f);
// 	}
// 
// 	ForceNetUpdate();
// }
// 
// void ANPC::OnRep_Hibernation()
// {
// 	HibernationModeChanged();
// }

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANPC::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANPC::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANPC::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANPC::LookUpAtRate);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANPC::JumpOrFly);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ANPC::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ANPC::ZoomOut);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANPC::Dismount);
	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ANPC::Server_LeftClickDown);
	PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &ANPC::Server_LeftClickUp);
	//PlayerInputComponent->BindAction("ShiftKey", IE_Pressed, this, &ANPC::Server_MountRun);
}

void ANPC::Server_LeftClickDown_Implementation()
{
	LeftClickDown = true;
}

bool ANPC::Server_LeftClickDown_Validate()
{
	return true;
}

void ANPC::Server_LeftClickUp_Implementation()
{
	LeftClickDown = false;
}

bool ANPC::Server_LeftClickUp_Validate()
{
	return true;
}


void ANPC::ZoomIn()
{
// 	if (CameraBoom->TargetArmLength > 300.f)
// 	{
// 		CameraBoom->TargetArmLength -= 100.f;
// 	}
}

void ANPC::ZoomOut()
{
// 	if (CameraBoom->TargetArmLength < 3000.f)
// 	{
// 		CameraBoom->TargetArmLength += 100.f;
// 	}
}

void ANPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UNavigationSystemV1::UnregisterNavigationInvoker(*this);

	for (AVeritexCharacter* Char : PlayersWhoAttackedUs)
	{
		if (Char != nullptr)
		{
			Char->Client_InvokeBattleTheme(false);
		}
	}

	if (TheSpawnerThatSpawnedUs)
	{
		TheSpawnerThatSpawnedUs->RandomLocationsToSpawnNPCs.Add(
			TheSpawnerThatSpawnedUs->RandomPointWithinVolumeGrounded());
		TheSpawnerThatSpawnedUs->AI_In_This_Biome.Remove(this);
	}

	PossessOriginalPawn();
	Super::EndPlay(EndPlayReason);
}

void ANPC::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetViewTargetWithBlend(this);
	}
}

void ANPC::UnPossessed()
{
	Super::UnPossessed();

	GetCharacterMovement()->StopMovementImmediately();

	Server_DismountAndMoveLeftOrRight();
}

void ANPC::Multicast_PlayBloodParticle_Implementation()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), P_BloodParticle, GetActorLocation(), FRotator::ZeroRotator,
		true);
}

bool ANPC::Multicast_PlayBloodParticle_Validate()
{
	return true;
}

float ANPC::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (DamageCauser && DamageCauser->IsA(StaticClass()))
	{
		return 0;
	}

	AVeritexCharacter* VAC = Cast<AVeritexCharacter>(DamageCauser);

	if (VAC)
	{
		VAC->Client_InvokeBattleTheme(true);
		PlayersWhoAttackedUs.Add(VAC);
	}
	//SetHibernate(false);

	float realDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	Health -= realDamage;

	Multicast_PlayBloodParticle();

	if (Health <= 0)
	{
		Health = 0;
		if (!bExperienceGivenFromDeath)
		{
			AVeritexCharacter* Char = Cast<AVeritexCharacter>(DamageCauser);

			if (Char != nullptr)
			{
				bExperienceGivenFromDeath = true;
				Char->ReceiveExperience(ExperienceToGive);
			}
		}

		if (!IsPendingKill())
		{
			if (!bLootBagSpawned)
			{
				bLootBagSpawned = true;
				PlayDeathAnimationAndDestroy();
			}
		}
	}

	if (TakeDamageMontage)
	{
		if (!GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
		{
			Multicast_PlayTakeDamageAnimation();
		}
	}

	// Notify Bot Behavior
	AVeritexAIController* AI = Cast<AVeritexAIController>(GetController());
	if (AI)
	{
		if (AI->GetBlackboardComponent())
		{
			UBlackboardComponent* BB = AI->GetBlackboardComponent();
			BB->SetValueAsObject(TEXT("CharacterToFollow"), DamageCauser);
			BB->SetValueAsBool(TEXT("ShouldRunFromPlayer?"), false);
		}
	}


	return realDamage;
}

void ANPC::PossessOriginalPawn()
{
	AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(GetController());
	if (VPC)
	{
		if (VPC->MyCharacter)
		{
			VPC->Possess(VPC->MyCharacter);
		}
	}
}

void ANPC::Multicast_PlayTakeDamageAnimation_Implementation()
{
	GetMesh()->GetAnimInstance()->Montage_Play(TakeDamageMontage);
}

bool ANPC::Multicast_PlayTakeDamageAnimation_Validate()
{
	return true;
}

void ANPC::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		//ensure(OriginalAnimInstance &&	"Make sure you set the OriginalAnimInstance for object pooling to take full affect on this NPC.");
		SetLifeSpan(1800.f); // Kill after 30 minutes
		SetStats();
	}
}

void ANPC::UpdateRateParams(FAnimUpdateRateParameters* Parameters)
{
	Parameters->bShouldUseLodMap = false;
	Parameters->BaseVisibleDistanceFactorThesholds.Reset();
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.600f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.400f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.200f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.100f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.050f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.020f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.001f);
	Parameters->BaseVisibleDistanceFactorThesholds.Add(0.000f);

	// Every threshold gets interpolation but last one
	Parameters->MaxEvalRateForInterpolation = Parameters->BaseVisibleDistanceFactorThesholds.Num();
}

void ANPC::SetStats()
{
	if (HasAuthority())
	{
		GetMesh()->SetPhysMaterialOverride(BloodPhysMaterial);
		//GetMesh()->bEnableUpdateRateOptimizations = true;

		GetCapsuleComponent()->SetPhysMaterialOverride(BloodPhysMaterial);

		NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		UNavigationSystemV1::RegisterNavigationInvoker(*this, 3000.f, 8000.f);

		if (MaxMovementSpeed == 0)
		{
			MaxMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
		}

		GetCharacterMovement()->MaxWalkSpeed = MaxMovementSpeed;
		GetCharacterMovement()->MaxCustomMovementSpeed = MaxMovementSpeed;

		if (Level == 0)
		{
			Level = FMath::RandRange(1, MaxLevel);
		}

		bIsDead = false;

		switch (MountName)
		{
		case EMountName::Raft:
			AnimalName = "Raft";
			BaseDamage = 0;
			Health = 50000;
			MaxHealth = Health;
			break;

		case EMountName::Boar:
			AnimalName = "Boar";
			BaseDamage = Level + 5;
			Health = Level * 2;

			NPCType = ENPCType::Carnivore;
			Torpor = Level * 35;

			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Bear:
			AnimalName = "Bear";
			BaseDamage = Level + 1;
			Health = Level * 2;

			NPCType = ENPCType::Carnivore;
			Torpor = Level * 35;

			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Cat:
			AnimalName = "Cat";
			BaseDamage = 0;
			Health = Level * 2;

			NPCType = ENPCType::Omnivore;
			Torpor = Level * 10;

			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Chicken:
			AnimalName = "Chicken";
			BaseDamage = 0;
			Health = Level * 3;

			NPCType = ENPCType::Omnivore;
			Torpor = Level * 5;

			Food = Level * Health;
			Water = Food;

			break;

		case EMountName::Cow:
			AnimalName = "Cow";
			BaseDamage = Level + 20;

			Health = Level + 15;

			NPCType = ENPCType::Herbivore;
			Torpor = Level * 50;

			Food = Level * Health;
			Water = Food;
			break;


		case EMountName::Crocodile:
			AnimalName = "Crocodile";
			BaseDamage = Level + 20;

			Health = Level + 15;

			NPCType = ENPCType::Carnivore;
			Torpor = Level * 50;

			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Deer:
			AnimalName = "Deer";
			BaseDamage = 0;

			Health = Level + 20;

			NPCType = ENPCType::Herbivore;
			Torpor = Level * 3;
			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Dog:
			AnimalName = "Dog";
			BaseDamage = 20;

			Health = Level * 5;

			NPCType = ENPCType::Carnivore;
			Torpor = Level * 5;
			Food = Level * Health;
			Water = Food;
			break;

		case EMountName::Elephant:
			AnimalName = "Elephant";
			BaseDamage = 100;

			Health = Level * 100;

			NPCType = ENPCType::Herbivore;
			Torpor = Level * 150;
			Food = Level * 80;
			Water = Food;
			break;

		case EMountName::Fox:
			AnimalName = "Fox";
			BaseDamage = 5;

			Health = Level * 2;

			NPCType = ENPCType::Omnivore;
			Torpor = Level * 2;
			Food = Level * 5;
			Water = Food;
			break;

		case EMountName::Goat:
			AnimalName = "Goat";
			BaseDamage = 5;

			Health = Level * 2;

			NPCType = ENPCType::Herbivore;
			Torpor = Level * 3;
			Food = Level * 5;
			Water = Food;

			break;

		case EMountName::Pig:
			// Pigs very fat, high health :^)
			AnimalName = "Pig";
			BaseDamage = 25;

			Health = Level * 2;

			NPCType = ENPCType::Omnivore;
			Torpor = Level * Level;
			Food = Torpor;
			Water = Torpor;

			break;

		case EMountName::Rabbit:
			AnimalName = "Rabbit";
			BaseDamage = 0;

			Health = Level * 1;

			NPCType = ENPCType::Omnivore;
			Torpor = Health;
			Food = Health;
			Water = Health;

			break;

		case EMountName::Sheep:
			AnimalName = "Sheep";
			BaseDamage = 10;

			Health = Level * 3;

			NPCType = ENPCType::Herbivore;
			Torpor = Level * 2;
			Food = Torpor;
			Water = Torpor;

			break;

		case EMountName::Wolf:
			AnimalName = "Wolf";
			BaseDamage = 15;

			Health = Level + 7;
			NPCType = ENPCType::Carnivore;
			Torpor = Level * 10;
			Food = Torpor;
			Water = Torpor;

			break;
		}

		MaxHealth = Health;
		MaxTorpor = Torpor;
		MaxWater = Water;
		MaxFood = Food;
		ExperienceToGive = Level / 3;
	}
}

void ANPC::ReleasedFromPool(FTransform const& Transform)
{
	RegisterAllActorTickFunctions(true, true);
	if (GetController())
	{
		GetController()->RegisterAllActorTickFunctions(true, true);
		AVeritexAIController* VAIC = Cast<AVeritexAIController>(GetController());
		if (VAIC)
		{
			VAIC->RunBehaviorTree(BotBehavior);
		}
	}
	//SetNetDormancy(DORM_Awake);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetVisibility(true);
	//GetMesh()->SetAnimInstanceClass(OriginalAnimInstance);
	Multicast_ReleasedFromPool();
}

void ANPC::Multicast_ReleasedFromPool_Implementation()
{
	RegisterAllActorTickFunctions(true, true);
	if (GetController())
	{
		GetController()->RegisterAllActorTickFunctions(true, true);
		AVeritexAIController* VAIC = Cast<AVeritexAIController>(GetController());
		if (VAIC)
		{
			VAIC->RunBehaviorTree(BotBehavior);
		}
	}
	//SetNetDormancy(DORM_Awake);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetVisibility(true);
	//GetMesh()->SetAnimInstanceClass(OriginalAnimInstance);
}

bool ANPC::Multicast_ReleasedFromPool_Validate()
{
	return true;
}

void ANPC::ReturnedToPool()
{
	RegisterAllActorTickFunctions(false, true);
	if (GetController())
	{
		GetController()->RegisterAllActorTickFunctions(false, true);
		AVeritexAIController* VAIC = Cast<AVeritexAIController>(GetController());
		if (VAIC)
		{
			VAIC->BehaviorComp->StopLogic("This actor has been pooled.");
		}
	}
	//SetNetDormancy(DORM_DormantAll);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetVisibility(false);
	//GetMesh()->SetAnimInstanceClass(nullptr);
	Health = MaxHealth;
	Multicast_ReturnedToPool();
}


void ANPC::Multicast_ReturnedToPool_Implementation()
{
	RegisterAllActorTickFunctions(false, true);
	if (GetController())
	{
		GetController()->RegisterAllActorTickFunctions(false, true);
		AVeritexAIController* VAIC = Cast<AVeritexAIController>(GetController());
		if (VAIC)
		{
			VAIC->BehaviorComp->StopLogic("This actor has been pooled.");
		}
	}
	//SetNetDormancy(DORM_DormantAll);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetVisibility(false);
	//GetMesh()->SetAnimInstanceClass(nullptr);
	Health = MaxHealth;
}

bool ANPC::Multicast_ReturnedToPool_Validate()
{
	return true;
}


void ANPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANPC, Health);
	DOREPLIFETIME(ANPC, MaxHealth);
	DOREPLIFETIME(ANPC, Level);
	//DOREPLIFETIME(ANPC, Torpor);
	//DOREPLIFETIME(ANPC, MaxTorpor);
	//DOREPLIFETIME(ANPC, Food);
	//DOREPLIFETIME(ANPC, MaxFood);
	//DOREPLIFETIME(ANPC, Water);
	//DOREPLIFETIME(ANPC, MaxWater);
	//DOREPLIFETIME(ANPC, bIsDead);
	//DOREPLIFETIME(ANPC, BaseDamage);
	DOREPLIFETIME(ANPC, AnimalName);
	//DOREPLIFETIME(ANPC, bIsHibernating);

	//DOREPLIFETIME(ANPC, OwnedByTribeID);
	//DOREPLIFETIME(ANPC, TribeRideRank);
	//DOREPLIFETIME(ANPC, MovementSpeed);
	//DOREPLIFETIME(ANPC, Stamina);
	//DOREPLIFETIME(ANPC, MountedCharacter);
	//DOREPLIFETIME(ANPC, LeftClickDown);

	DOREPLIFETIME(ANPC, OwnedBy);
}

void ANPC::Dismount()
{
	Server_Dismount();
}

void ANPC::Server_DismountAndMoveLeftOrRight_Implementation()
{
#if WITH_SERVER_CODE
	if (MountedCharacter != nullptr)
	{
		FVector NewLocation = MountedCharacter->GetActorLocation() + (MountedCharacter->GetActorRightVector() *
			DismountAmount);
		MountedCharacter->SetActorLocation(NewLocation);

		if (MountedCharacter)
		{
			MountedCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			GetCharacterMovement()->StopMovementImmediately();
		}
	}
#endif
}

bool ANPC::Server_DismountAndMoveLeftOrRight_Validate()
{
	return true;
}

void ANPC::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void ANPC::Server_SetMovementMode_Implementation(EMovementMode NewMode)
{
#if WITH_SERVER_CODE
	GetCharacterMovement()->SetMovementMode(NewMode);
#endif
}

bool ANPC::Server_SetMovementMode_Validate(EMovementMode NewMode)
{
	return true;
}

void ANPC::Server_Dismount_Implementation()
{
	AVeritexPlayerController* VPC = Cast<AVeritexPlayerController>(GetController());
	if (VPC)
	{
		VPC->MyCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		VPC->UnPossess();
		// Set position to right above the raft?
		//VPC->MyCharacter->GetRootComponent()->RecreatePhysicsState();
		VPC->MyCharacter->SetActorLocation(GetActorLocation() + (GetActorUpVector() * 100));
		VPC->Possess(VPC->MyCharacter);
	}
}

bool ANPC::Server_Dismount_Validate()
{
	return true;
}


void ANPC::JumpOrFly()
{
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();

	if (CanMountFly())
	{
		if (CharMovement->MovementMode != MOVE_Flying)
		{
			//CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
			Server_SetMovementMode(MOVE_Flying);
		}
		if (CharMovement->MovementMode == MOVE_Flying)
		{
			//CharMovement->SetMovementMode(EMovementMode::MOVE_Walking);
			Server_SetMovementMode(MOVE_Walking);
		}
	}
	else if (CanMountJump())
	{
		Jump();
	}
}

bool ANPC::CanMountFly()
{
	switch (MountName)
	{
	case EMountName::Bear:
		return false;
	case EMountName::Deer:
		return false;
	case EMountName::Griffin:
		return true;
	case EMountName::Pegasus:
		return true;
	case EMountName::Phoenix:
		return true;
	case EMountName::SkyWhale:
		return true;
	case EMountName::Dragon:
		return true;

	default:
		return false;
	}
}

bool ANPC::CanMountJump()
{
	switch (MountName)
	{
	case EMountName::Phoenix:
		return false;
	default:
		return false;
	}
}

void ANPC::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANPC::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (GetCharacterMovement()->MovementMode == MOVE_Flying)
		{
			//AddMovementInput(FollowCamera->GetForwardVector(), Value);
		}
		else
		{
			AddMovementInput(Direction, Value);
		}
	}
}

void ANPC::MoveRight(float Value)
{
	switch (MountName)
	{
	case EMountName::Dragon:

		break;
	default:
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction

			AddMovementInput(Direction, Value);
		}
		break;
	}
}

// bool ANPC::IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer)
// {
// 	if (NetEnablePauseRelevancy == 1)
// 	{
// 		APlayerController* PC = Cast<APlayerController>(ConnectionOwnerNetViewer.InViewer);
// 		check(PC);
// 
// 		FVector ViewLocation;
// 		FRotator ViewRotation;
// 		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
// 
// 		FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(LineOfSight), true, PC->GetPawn());
// 		CollisionParams.AddIgnoredActor(this);
// 
// 		TArray<FVector> PointsToTest;
// 		BuildPauseReplicationCheckPoints(PointsToTest);
// 
// 		for (FVector PointToTest : PointsToTest)
// 		{
// 			if (!GetWorld()->LineTraceTestByChannel(PointToTest, ViewLocation, ECC_Visibility, CollisionParams))
// 			{
// 
// 				return false;
// 			}
// 		}
// 
// 		return true;
// 	}
// 
// 	return false;
// }
// 
// void ANPC::BuildPauseReplicationCheckPoints(TArray<FVector>& RelevancyCheckPoints)
// {
// 	FBoxSphereBounds Bounds = GetCapsuleComponent()->CalcBounds(GetCapsuleComponent()->GetComponentTransform());
// 	FBox BoundingBox = Bounds.GetBox();
// 	float XDiff = Bounds.BoxExtent.X * 2;
// 	float YDiff = Bounds.BoxExtent.Y * 2;
// 
// 	RelevancyCheckPoints.Add(BoundingBox.Min);
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Min.X + XDiff, BoundingBox.Min.Y, BoundingBox.Min.Z));
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Min.X, BoundingBox.Min.Y + YDiff, BoundingBox.Min.Z));
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Min.X + XDiff, BoundingBox.Min.Y + YDiff, BoundingBox.Min.Z));
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Max.X - XDiff, BoundingBox.Max.Y, BoundingBox.Max.Z));
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Max.X, BoundingBox.Max.Y - YDiff, BoundingBox.Max.Z));
// 	RelevancyCheckPoints.Add(FVector(BoundingBox.Max.X - XDiff, BoundingBox.Max.Y - YDiff, BoundingBox.Max.Z));
// 	RelevancyCheckPoints.Add(BoundingBox.Max);
// }
// 
// void ANPC::OnReplicationPausedChanged(bool bIsReplicationPaused)
// {
// 	//GetMesh()->SetHiddenInGame(bIsReplicationPaused, false);
// 	// Removing this code because it is causing some NPCs to turn invisible when even on screen...
// }
// 
