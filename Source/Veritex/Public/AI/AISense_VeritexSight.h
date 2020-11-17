// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Perception/AISense_Sight.h"
#include "AISense_VeritexSight.generated.h"

/**
 * 
 */
UCLASS()
class VERITEX_API UAISense_VeritexSight : public UAISense_Sight
{
	GENERATED_BODY()

		UAISense_VeritexSight();
	
protected:
	//virtual bool RegisterTarget(AActor& TargetActor, FQueriesOperationPostProcess PostProcess) override;

};
