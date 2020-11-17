// Daniel Gleason (C) 2017

#pragma once

#include "WaterCraftInterface.generated.h"

UINTERFACE(Blueprintable)
class VERITEX_API UWaterCraftInterface : public UInterface
{
	GENERATED_BODY()
};


class IWaterCraftInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Water Craft")
		bool IsWaterCraft() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Water Craft")
		bool IsPossessable() const;


};