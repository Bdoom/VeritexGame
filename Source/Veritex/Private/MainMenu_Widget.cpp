// Daniel Gleason (C) 2017

#include "MainMenu_Widget.h"

void UMainMenu_Widget::SetLocalization(FString newLocale)
{
	FInternationalization::Get().SetCurrentCulture(newLocale);
}
