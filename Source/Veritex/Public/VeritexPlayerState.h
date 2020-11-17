// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/PlayerState.h"

#include "VeritexCharacter.h"

#include "VeritexPlayerState.generated.h"


/**
 * 
 */
UCLASS()
class VERITEX_API AVeritexPlayerState : public APlayerState
{
	GENERATED_BODY()

	AVeritexPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	
public:
	UPROPERTY(BlueprintReadWrite, Category = OurCharacter)
		AVeritexCharacter* MyCharacter;

};
