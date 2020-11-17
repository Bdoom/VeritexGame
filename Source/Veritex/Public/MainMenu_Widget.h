// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu_Widget.generated.h"

/**
 *
 */
UCLASS(Config = Game)
class VERITEX_API UMainMenu_Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadWrite)
		FString WebURL;

	UFUNCTION(BlueprintCallable)
		void SetLocalization(FString newLocale);


};
