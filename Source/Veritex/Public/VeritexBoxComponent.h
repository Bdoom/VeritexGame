// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "VeritexStructs.h"
#include "VeritexBoxComponent.generated.h"

/**
 * Class is the same as a regular UBoxComponent. However, it has an added EStructureType used to snap structures to their appropriate structure snapping slots. 
 * This class should be used instead of a regular UBoxComponent if intended to be used for snapping purposes. 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class VERITEX_API UVeritexBoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Structures)
		EStructureType StructureType;

	UVeritexBoxComponent();

	
};
