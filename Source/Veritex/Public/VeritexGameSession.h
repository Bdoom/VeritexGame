// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/GameSession.h"
#include "VeritexGameSession.generated.h"

/**
 * 
 */
UCLASS()
class VERITEX_API AVeritexGameSession : public AGameSession
{
	GENERATED_BODY()

	virtual void RegisterServer() override;
	
	
public:

	int MyMaxPlayers = 150;

	//TSharedPtr<class FOnlineSessionSettings> HostSettings;

	
};
