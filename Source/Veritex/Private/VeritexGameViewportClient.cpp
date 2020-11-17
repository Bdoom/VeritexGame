// Daniel Gleason (C) 2017

#include "VeritexGameViewportClient.h"

#include "Veritex.h"
#include "Blueprint/UserWidget.h"


void UVeritexGameViewportClient::ShowLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		return;
	}

	LoadingScreenWidgetClass = LoadObject<UClass>(nullptr, TEXT("/Game/Blueprints/LoadingScreen.LoadingScreen"), nullptr, 0, nullptr);
	if (LoadingScreenWidgetClass != NULL)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(GetGameInstance(), LoadingScreenWidgetClass);
		AddViewportWidgetContent(LoadingScreenWidget->TakeWidget());
	}
}

void UVeritexGameViewportClient::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		RemoveViewportWidgetContent(LoadingScreenWidget->TakeWidget());
		LoadingScreenWidget = NULL;
	}
}
