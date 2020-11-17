// Daniel Gleason (C) 2017

#include "VeritexSpell.h"

#include "Veritex.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"


// Sets default values
AVeritexSpell::AVeritexSpell()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->InitSphereRadius(250.f);
	RootComponent = SphereComponent;

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System Component"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());

	bReplicates = true;
	bReplicateMovement = true;
	ConsistentDamage = false;
	BaseDamage = 10.f;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AVeritexSpell::OnComponentBeginOverlap);

	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->ProjectileGravityScale = 0.1f;
	ProjectileMovement->Velocity = FVector(1, 0, 0);

	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECollisionResponse::ECR_Block);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> Blood(TEXT("ParticleSystem'/Game/FXVillesBloodVFXPack/Particles/PS_Blood_Spurt_02.PS_Blood_Spurt_02'"));
	if (Blood.Object)
	{
		Blood_PS = Blood.Object;
	}

}

void AVeritexSpell::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (HasAuthority())
	{
		if (!ConsistentDamage)
		{
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);

			OtherActor->TakeDamage(BaseDamage, DamageEvent, nullptr, this);

			if (OtherActor->IsA(ACharacter::StaticClass()))
			{
				Multicast_SpawnBloodParticle(OtherActor);
			}
		}
	}
}

void AVeritexSpell::Multicast_SpawnBloodParticle_Implementation(AActor* OtherActor)
{
	if (!OtherActor)
	{
		return;
	}
	if (Blood_PS == NULL)
	{
		return;
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Blood_PS, OtherActor->GetActorTransform());
}

bool AVeritexSpell::Multicast_SpawnBloodParticle_Validate(AActor* OtherActor)
{
	return true;
}


// Called when the game starts or when spawned
void AVeritexSpell::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (ConsistentDamage)
		{
			FTimerHandle ConsistentDamageHandle;
			GetWorld()->GetTimerManager().SetTimer(ConsistentDamageHandle, this, &AVeritexSpell::DealSpellDamage, ConsistentDamageTimer, true);
		}
	}

}

void AVeritexSpell::DealSpellDamage()
{
	TArray<AActor*> ActorsOverlapping;
	SphereComponent->GetOverlappingActors(ActorsOverlapping);
	for (AActor* TheActor : ActorsOverlapping) 
	{
		if (TheActor)
		{
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);

			TheActor->TakeDamage(BaseDamage, DamageEvent, nullptr, this);

			if (TheActor->IsA(ACharacter::StaticClass()))
			{
				Multicast_SpawnBloodParticle(TheActor);
			}
		}
	}
}

void AVeritexSpell::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVeritexSpell, BaseDamage);
	DOREPLIFETIME(AVeritexSpell, HasOverlappingActor);
}

