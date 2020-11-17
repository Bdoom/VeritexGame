// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"

#include "VeritexAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class ANPC;

//DECLARE_STATS_GROUP(TEXT("VeritexAIController"), STATGROUP_VeritexAIController, STATCAT_Advanced);
//DECLARE_CYCLE_STAT(TEXT("VeritexAIController - OnPerceptionTargetUpdated"), STAT_VeritexPerceptionUpdate, STATGROUP_VeritexAIController);


UCLASS()
class VERITEX_API AVeritexAIController : public AAIController
{
	GENERATED_BODY()

public:

	AVeritexAIController(const FObjectInitializer& ObjectInitializer);

// 	UFUNCTION()
// 		bool IsHibernationModeEnabled() { return bIsHibernating; }
// 
// 	UFUNCTION()
// 		void OnHibernationModeChanged();
// 
// 	UFUNCTION()
// 		void SetHibernate(bool bHibernate);
// 
// 	UPROPERTY()
// 		bool bIsHibernating = false;

	//UFUNCTION()
		//void OnTargetPerceptionUpdated(AActor* TheActor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable, NetMulticast, WithValidation, Unreliable)
		void Multicast_PlaySoundWhenPerceived();

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		//UAISenseConfig_Damage* DamageConfig;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		//UAISenseConfig_Sight* SightConfig;

	UPROPERTY(transient)
		UBlackboardComponent* BlackboardComp;

	/* Cached BT component */
	UPROPERTY(transient)
		UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(BlueprintReadWrite, Category = Perception)
		ANPC* LastPerceiecedNPC;

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
