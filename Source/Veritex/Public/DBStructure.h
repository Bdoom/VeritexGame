// Daniel Gleason (C) 2017

#pragma once

#include "UObject/NoExportTypes.h"
#include "DBStructure.generated.h"

/**
 * 
 */
UCLASS()
class VERITEX_API UDBStructure : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
		float Structure_Location_X;

	UPROPERTY()
		float Structure_Location_Y;

	UPROPERTY()
		float Structure_Location_Z;

	UPROPERTY()
		float Structure_Rotation_X;

	UPROPERTY()
		float Structure_Rotation_Y;

	UPROPERTY()
		float Structure_Rotation_Z;
	
	UPROPERTY()
		FString PlayerName;

	UPROPERTY()
		FString TribeName;

	UPROPERTY()
		float Health;

	UPROPERTY()
		float MaxHealth;

	UPROPERTY()
		FString OwnedBySteamID;

	UPROPERTY()
		FString StructurePath;

	
	
};
