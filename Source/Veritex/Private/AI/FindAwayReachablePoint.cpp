// Daniel Gleason (C) 2017

#include "FindAwayReachablePoint.h"
#include "Veritex.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "NavigationSystem.h"
#include "VeritexAIController.h"
#include "NPC.h"


UFindAwayReachablePoint::UFindAwayReachablePoint(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Find Away Reachable Point";

	Radius = 300;
	Distance = 600;

	Origin.SelectedKeyName = TEXT("SelfActor");
	From.SelectedKeyName = TEXT("SelfActor");
	Result.SelectedKeyName = TEXT("MoveTo");
}

EBTNodeResult::Type UFindAwayReachablePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	class AAIController* const AI = OwnerComp.GetAIOwner();
	class APawn* const Pawn = AI ? AI->GetPawn() : nullptr;

	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	FVector OriginLocation = FVector::ZeroVector;
	if (!BB->GetLocationFromEntry(Origin.GetSelectedKeyID(), OriginLocation))
	{
		OriginLocation = Pawn->GetActorLocation();
	}

	FVector FromLocation = FVector::ZeroVector;
	if (!BB->GetLocationFromEntry(From.GetSelectedKeyID(), FromLocation))
	{
		AVeritexAIController* VAIC = Cast<AVeritexAIController>(Pawn->GetController());
		if (VAIC)
		{
			if (VAIC->LastPerceiecedNPC)
			{
				FromLocation = VAIC->LastPerceiecedNPC->GetActorLocation();
				
			}
			else
			{
				return EBTNodeResult::Failed;
			}
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}

	const float DistanceFrom = (OriginLocation - FromLocation).Size();

	const FVector SearchOrigin = FromLocation + (Pawn->GetActorLocation() - FromLocation) * (DistanceFrom + Radius + Distance);

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation ResultLocation;
	NavSystem->GetRandomReachablePointInRadius(SearchOrigin, Radius, ResultLocation);
	const FVector ReachablePoint = ResultLocation.Location;

	if (!ReachablePoint.IsNearlyZero())
	{
		BB->SetValue<UBlackboardKeyType_Vector>(Result.GetSelectedKeyID(), ReachablePoint);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

void UFindAwayReachablePoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	Origin.ResolveSelectedKey(*GetBlackboardAsset());
	From.ResolveSelectedKey(*GetBlackboardAsset());
	Result.ResolveSelectedKey(*GetBlackboardAsset());

}
