// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/PlayerStart.h"
#include "VeritexPlayerStart.generated.h"

UENUM(BlueprintType)
enum class EBiomeZone : uint8
{
	LifeIsland,
	DeathIsland,
	IronIsland,
	MainIsland,
	None
};


UCLASS()
class VERITEX_API AVeritexPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Biome)
		EBiomeZone BiomeZone;
	
};
