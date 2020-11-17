// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "VeritexWorldSettings.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class VERITEX_API AVeritexWorldSettings : public AWorldSettings
{
	GENERATED_BODY()


public:

	AVeritexWorldSettings();

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, meta = (ClampMin = 0, ClampMax = 2400, UIMin = 0, UIMax = 2400), Category = TimeOfDay)
		float TimeOfDay;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = TimeOfDay)
		float MoonLightIntensity;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = TimeOfDay)
		float SunLightIntensity;

	UFUNCTION()
		bool IsNightTime();

	UFUNCTION()
		bool IsDayTime();

	UFUNCTION()
		bool IsMidDay();

	virtual void NotifyBeginPlay() override;

private:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

};
