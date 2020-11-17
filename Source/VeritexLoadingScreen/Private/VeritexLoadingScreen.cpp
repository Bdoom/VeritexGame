// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VeritexLoadingScreen.h"
#include "GenericApplication.h"
#include "GenericApplicationMessageHandler.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"

struct FVeritexLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FVeritexLoadingScreenBrush(const FName InTextureName, const FVector2D& InImageSize)
		: FSlateDynamicImageBrush(InTextureName, InImageSize)
	{
		ResourceObject = LoadObject<UObject>(NULL, *InTextureName.ToString());
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if (ResourceObject)
		{
			Collector.AddReferencedObject(ResourceObject);
		}
	}
};

class SVeritexLoadingScreenSlateWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVeritexLoadingScreenSlateWidget) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
	{
		static const FName LoadingScreenName(TEXT("/Game/Blueprints/LoadingScreen.LoadingScreen"));

		//since we are not using game styles here, just load one image
		LoadingScreenBrush = MakeShareable(new FVeritexLoadingScreenBrush(LoadingScreenName, FVector2D(1920, 1080)));

		ChildSlot
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(LoadingScreenBrush.Get())
			]
		+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SSafeZone)
				.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Padding(10.0f)
			.IsTitleSafe(true)
			[
				SNew(SThrobber)
				.Visibility(this, &SVeritexLoadingScreenSlateWidget::GetLoadIndicatorVisibility)
			]
			]
			];
	}

private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return EVisibility::Visible;
	}

	/** loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
};


// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

class FVeritexLoadingScreen : public IVeritexLoadingScreen
{
public:
	
	virtual void StartupModule() override
	{
		// Load for cooker reference
		LoadObject<UObject>(NULL, TEXT("/Game/Blueprints/LoadingScreen.LoadingScreen"));
// 		
// 		if (IsMoviePlayerEnabled())
// 		{
// 			GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, &FVeritexLoadingScreen::StartLoadingScreen);
// 
// 			FLoadingScreenAttributes LoadingScreen;
// 			LoadingScreen.MinimumLoadingScreenDisplayTime = 30.f;
// 			LoadingScreen.bMoviesAreSkippable = false;
// 			LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
//			LoadingScreen.MoviePaths.Add(TEXT("UE4_Logo"));
// 			LoadingScreen.MoviePaths.Add(TEXT("veritex_intro_shipping"));
// 			
// 			GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
// 		}
	}

	virtual void StartLoadingScreen() override
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		LoadingScreen.bMoviesAreSkippable = false;
		LoadingScreen.bWaitForManualStop = false;

		LoadingScreen.WidgetLoadingScreen = SNew(SVeritexLoadingScreenSlateWidget);
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
	
	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void HideLoadingScreen() override
	{
		GetMoviePlayer()->StopMovie();
	}
	
};

IMPLEMENT_GAME_MODULE(FVeritexLoadingScreen, VeritexLoadingScreen);

