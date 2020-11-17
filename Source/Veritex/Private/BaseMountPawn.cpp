// Daniel Gleason (C) 2017

#include "BaseMountPawn.h"
#include "Veritex.h"
#include "BaseMountPawnMovement.h"
#include "VeritexPlayerController.h"
#include "CoreMinimal.h"

// Sets default values
ABaseMountPawn::ABaseMountPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMountPawnMovement>(ACharacter::CharacterMovementComponentName))
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 450.0f; //600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetCharacterMovement()->MaxFlySpeed = 0;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;
	bReplicateMovement = true;

}


void ABaseMountPawn::Server_DismountAndMoveLeftOrRight_Implementation()
{
#if WITH_SERVER_CODE
	if (MountedCharacter != nullptr)
	{
		FVector NewLocation = MountedCharacter->GetActorLocation() + (MountedCharacter->GetActorRightVector() * DismountAmount);
		MountedCharacter->SetActorLocation(NewLocation);
	}
#endif
}

bool ABaseMountPawn::Server_DismountAndMoveLeftOrRight_Validate()
{
	return true;
}

// Called when the game starts or when spawned
void ABaseMountPawn::BeginPlay()
{
	Super::BeginPlay();
	switch (MountName)
	{
	case EMountName::Phoenix:
		CameraBoom->TargetArmLength = 450.f;
		break;

	case EMountName::Dragon:
		CameraBoom->TargetArmLength = 3000.f;
		DismountAmount = 450.f;
		break;

	default:
		CameraBoom->TargetArmLength = 450.f;
		break;
	}

}

// Called every frame
void ABaseMountPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseMountPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseMountPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseMountPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABaseMountPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseMountPawn::LookUpAtRate);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABaseMountPawn::JumpOrFly);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}

void ABaseMountPawn::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABaseMountPawn::Server_SetMovementMode_Implementation(EMovementMode NewMode)
{
#if WITH_SERVER_CODE
	GetCharacterMovement()->SetMovementMode(NewMode);
#endif
}

bool ABaseMountPawn::Server_SetMovementMode_Validate(EMovementMode NewMode)
{
	return true;
}


void ABaseMountPawn::JumpOrFly()
{
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();

	if (CanMountFly())
	{

		if (CharMovement->MovementMode != EMovementMode::MOVE_Flying)
		{
			//CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
			Server_SetMovementMode(MOVE_Flying);
		}
		if (CharMovement->MovementMode == EMovementMode::MOVE_Flying)
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

bool ABaseMountPawn::CanMountFly()
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

bool ABaseMountPawn::CanMountJump()
{
	switch (MountName)
	{
	case EMountName::Phoenix:
		return false;
	default:
		return false;
	}
}

void ABaseMountPawn::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABaseMountPawn::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
		{
			AddMovementInput(FollowCamera->GetForwardVector(), Value);
		}
		else
		{
			AddMovementInput(Direction, Value);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::FromInt(GetCharacterMovement()->MovementMode)); // 4 = flying, 3 == falling

	}
}

void ABaseMountPawn::MoveRight(float Value)
{
	switch (MountName)
	{
	case EMountName::Dragon:

		break;
	default:
		if ((Controller != NULL) && (Value != 0.0f))
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


void ABaseMountPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseMountPawn, OwnedByTribeID);
	DOREPLIFETIME(ABaseMountPawn, TribeRideRank);
	DOREPLIFETIME(ABaseMountPawn, Health);
	DOREPLIFETIME(ABaseMountPawn, MovementSpeed);
	DOREPLIFETIME(ABaseMountPawn, Stamina);
	DOREPLIFETIME(ABaseMountPawn, MountedCharacter);
}

float ABaseMountPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	Health -= damage;

	return damage;
}



