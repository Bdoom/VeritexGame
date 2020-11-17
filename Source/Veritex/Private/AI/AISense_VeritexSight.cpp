// Daniel Gleason (C) 2017

#include "AISense_VeritexSight.h"
#include "VeritexCharacter.h"

UAISense_VeritexSight::UAISense_VeritexSight()
{
	bAutoRegisterAllPawnsAsSources = false;
}

// Had to edit the engine code and add "virtual" identifier to RegisterTarget function


// bool UAISense_VeritexSight::RegisterTarget(AActor& TargetActor, FQueriesOperationPostProcess PostProcess)
// {
// 	GLog->Log("Number of observed targets: " + FString::FromInt(ObservedTargets.Num()));
// 	GLog->Log("New registered actor name: " + TargetActor.GetName());
// 
// 	// Without this code, the AISense will register ALL NPCS as potential enemies or observed targets.
// 	//return Super::RegisterTarget(TargetActor, PostProcess);
// 
// 	if (TargetActor.IsA(AVeritexCharacter::StaticClass()))
// 	{
// 		return Super::RegisterTarget(TargetActor, PostProcess);
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }
