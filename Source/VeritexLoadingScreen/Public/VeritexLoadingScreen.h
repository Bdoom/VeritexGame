#pragma once

#ifndef __VERITEXLOADINGSCREEN_H__
#define __VERITEXLOADINGSCREEN_H__

#include "ModuleInterface.h"

class IVeritexLoadingScreen : public IModuleInterface
{
public:

	virtual void StartLoadingScreen() = 0;
	virtual void HideLoadingScreen() = 0;

};

#endif
