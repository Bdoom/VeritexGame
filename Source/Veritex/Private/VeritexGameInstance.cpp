// Daniel Gleason (C) 2017

#include "VeritexGameInstance.h"
#include "MoviePlayer.h"

#include "Components/Widget.h"

// Slate
#include "Blueprint/UserWidget.h"
// End Slate

void UVeritexGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UVeritexGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UVeritexGameInstance::EndLoadingScreen);

	FCoreDelegates::OnExit.AddUObject(this, &UVeritexGameInstance::OnQuitGame);
}

void UVeritexGameInstance::OnQuitGame()
{
	OnQuit();
}

FString UVeritexGameInstance::GetIPAndPort()
{
	return GetWorld()->GetAddressURL();
}

void UVeritexGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (!IsRunningDedicatedServer())
	{
	}
}

void UVeritexGameInstance::EndLoadingScreen(UWorld* World)
{


	//GetMoviePlayer()->StopMovie();
}

TSharedRef<class SWidget> UVeritexGameInstance::LoadingScreenWidget()
{
	return LoadingScreenUserWidget->TakeWidget(); //SNew(LoadingScreenUserWidget->TakeWidget());
}

void UVeritexGameInstance::CreateLoadingScreenWidgetFromClass()
{
	LoadingScreenUserWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
}

void UVeritexGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

}
