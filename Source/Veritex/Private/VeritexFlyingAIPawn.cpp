// Daniel Gleason (C) 2017

#include "VeritexFlyingAIPawn.h"

#include "Veritex.h"

void AVeritexFlyingAIPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVeritexFlyingAIPawn, Health);
	DOREPLIFETIME(AVeritexFlyingAIPawn, MaxHealth);
}

void AVeritexFlyingAIPawn::BeginPlay()
{
	Super::BeginPlay();

	Health = 100;
	MaxHealth = Health;

}

float AVeritexFlyingAIPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float RealDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	Health -= RealDamage;

	if (Health <= 0)
	{
		Destroy();
	}

	return Damage;
}
