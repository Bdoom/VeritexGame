// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VeritexServerWidget.generated.h"

/**
 *
 */
UCLASS()
class VERITEX_API UVeritexServerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Connect to Server")
		void ConnectToServer(APlayerController* PC, FString IP);


};
