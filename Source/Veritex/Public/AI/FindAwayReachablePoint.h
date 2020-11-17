// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "FindAwayReachablePoint.generated.h"

/**
 *
 */
UCLASS()
class VERITEX_API UFindAwayReachablePoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UFindAwayReachablePoint(const class FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	UPROPERTY(EditAnywhere, Category = Blackboard)
		struct FBlackboardKeySelector Origin;

	UPROPERTY(EditAnywhere, Category = Blackboard)
		struct FBlackboardKeySelector From;

	UPROPERTY(EditAnywhere, Category = Blackboard)
		struct FBlackboardKeySelector Result;

	UPROPERTY(EditAnywhere, Category = Condition)
		float Radius;

	UPROPERTY(EditAnywhere, Category = Condition)
		float Distance;


};
