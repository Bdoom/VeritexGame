// Daniel Gleason (C) 2017

#include "VeritexServerWidget.h"

#include "Veritex.h"

void UVeritexServerWidget::ConnectToServer(APlayerController* PC, FString IP)
{
	if (PC)
	{
		PC->ClientTravel(IP, ETravelType::TRAVEL_Absolute);
	}
}
