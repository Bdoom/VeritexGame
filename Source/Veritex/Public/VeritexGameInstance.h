// Daniel Gleason (C) 2017

#pragma once

#include "Engine/GameInstance.h"

#include "Widgets/SWidget.h"

#include "VeritexGameInstance.generated.h"


UCLASS()
class VERITEX_API UVeritexGameInstance : public UGameInstance
{
	GENERATED_BODY()
	virtual void Init() override;

	UFUNCTION(BlueprintPure)
		FString GetIPAndPort();

	void OnQuitGame();

	UFUNCTION()
		virtual void BeginLoadingScreen(const FString& MapName);

	UFUNCTION()
		virtual void EndLoadingScreen(UWorld* World);

	TSharedRef<class SWidget> LoadingScreenWidget();

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "On Quit")
		void OnQuit();

	UPROPERTY(BlueprintReadWrite)
		class UWidget* LoadingScreenUserWidget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UFUNCTION()
		void CreateLoadingScreenWidgetFromClass();

	virtual void StartGameInstance() override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FString> OurMoviePaths;

};
