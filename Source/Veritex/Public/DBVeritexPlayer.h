// Daniel Gleason (C) 2017

#pragma once

#include "UObject/NoExportTypes.h"
#include "DBVeritexPlayer.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class VERITEX_API UDBVeritexPlayer : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
		float Player_Location_X;

	UPROPERTY(BlueprintReadWrite)
		float Player_Location_Y;

	UPROPERTY(BlueprintReadWrite)
		float Player_Location_Z;

	UPROPERTY(BlueprintReadWrite)
		FString UniqueSteamID;

	UPROPERTY(BlueprintReadWrite)
		FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
		FString TribeName;

	UPROPERTY(BlueprintReadWrite)
		FString TribeID;

	UPROPERTY(BlueprintReadWrite)
		int32 TribeRank;

	UPROPERTY(BlueprintReadWrite)
		int Health;

	UPROPERTY(BlueprintReadWrite)
		int MaxHealth;

	UPROPERTY(BlueprintReadWrite)
		int Food;

	UPROPERTY(BlueprintReadWrite)
		int MaxFood;

	UPROPERTY(BlueprintReadWrite)
		int Water;

	UPROPERTY(BlueprintReadWrite)
		int MaxWater;

	UPROPERTY(BlueprintReadWrite)
		FString RightHand;

	UPROPERTY(BlueprintReadWrite)
		FString LeftHand;

	UPROPERTY(BlueprintReadWrite)
		FString Head;

	UPROPERTY(BlueprintReadWrite)
		FString Backpack;

	UPROPERTY(BlueprintReadWrite)
		FString Feet;

	UPROPERTY(BlueprintReadWrite)
		FString Legs;

	UPROPERTY(BlueprintReadWrite)
		FString Chest;

	UPROPERTY(BlueprintReadWrite)
		float Experience;

	UPROPERTY(BlueprintReadWrite)
		int32 CurrentLevel;

	UPROPERTY(BlueprintReadWrite)
		int32 MaxWeight;

	UPROPERTY(BlueprintReadWrite)
		int32 MaxOxygen;

	UPROPERTY(BlueprintReadWrite)
		int32 MovementSpeed;

	UPROPERTY(BlueprintReadWrite)
		int32 MaxStamina;

	UPROPERTY(BlueprintReadWrite)
		int32 MeleeDamage;

	UPROPERTY(BlueprintReadWrite)
		int32 LevelUpPoints;

};
