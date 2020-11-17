// Daniel Gleason (C) 2017


#include "VeritexProjectile.h"
#include "Veritex.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

//https://i.imgur.com/fSFudSo.png
//https://i.imgur.com/WqkSe2L.png
// Differences?

// Sets default values
AVeritexProjectile::AVeritexProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));

	RootComponent = Collider;
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	//Collider->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));

	ProjectileMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Projectile->InitialSpeed = 3000.f;
	Projectile->MaxSpeed = 3000.f;
	Projectile->Velocity = GetActorForwardVector() * 3000.f;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AVeritexProjectile::BeginPlay()
{
	Super::BeginPlay();
	
// 	Projectile->InitialSpeed = 3000.f;
// 	Projectile->MaxSpeed = 3000.f;
// 	//Projectile->bShouldBounce = true;
// 	Projectile->Velocity = GetActorForwardVector() * 3000.f;
	//Projectile->bInitialVelocityInLocalSpace
}

// Called every frame
void AVeritexProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

