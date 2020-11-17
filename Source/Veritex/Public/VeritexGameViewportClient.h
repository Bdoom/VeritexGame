// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "VeritexGameViewportClient.generated.h"

/**
 *
 */
UCLASS()
class VERITEX_API UVeritexGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	UFUNCTION()
		void ShowLoadingScreen();

	UFUNCTION()
		void HideLoadingScreen();

	UPROPERTY()
		UUserWidget* LoadingScreenWidget;

	UPROPERTY()
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

};
