// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "VeritexFlyingAIPawn.generated.h"

UCLASS()
class VERITEX_API AVeritexFlyingAIPawn : public ADefaultPawn
{
	GENERATED_BODY()
	
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = FlyingAI)
		float Health;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = FlyingAI)
		float MaxHealth;
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

};
