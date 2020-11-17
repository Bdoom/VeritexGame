// Daniel Gleason (C) 2017

#pragma once

#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"

#include "CoreMinimal.h"


class FSaveLoadWorldTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FSaveLoadWorldTask>;

public:
	FSaveLoadWorldTask(bool bSaving, FWorldData WorldData) :
		bSaving(bSaving),
		WorldData(WorldData)
	{}
	FSaveLoadWorldTask(bool bSaving) :
		bSaving(bSaving)
	{}
	FSaveLoadWorldTask(bool bSaving, FString UniqueSteamID, FCharacterData CharacterData, bool IsCreativeMode) :
		bSaving(bSaving),
		UniqueSteamID(UniqueSteamID),
		CharacterData(CharacterData),
		IsCreativeMode(IsCreativeMode)
	{}
	FSaveLoadWorldTask(bool bSaving, FString UniqueSteamID, bool bKeepAlive, FVector SpawnLocation) :
		bSaving(bSaving),
		UniqueSteamID(UniqueSteamID),
		bKeepAlive(bKeepAlive),
		SpawnLocation(SpawnLocation)
	{}

protected:
	bool bSaving;
	FString UniqueSteamID;
	FCharacterData CharacterData;
	FWorldData WorldData;
	bool bKeepAlive;
	FVector SpawnLocation;
	bool IsCreativeMode;

	void SaveLoadData(FArchive& Ar, FWorldData& WorldDataLocal)
	{
		Ar << WorldDataLocal;
	}

	void SaveLoadCharacterData(FArchive& Ar, FCharacterData& Character)
	{
		Ar << Character;
	}

	void LoadCharacterFromUniqueSteamID();

	void SaveCharacterDataToFile();

	void LoadFromFile();

	void SaveToFile();

	void DoWork();


	// This next section of code needs to be here.  Not important as to why.
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSaveLoadWorldTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};


class VERITEX_API VeritexAsyncClasses
{
public:
	VeritexAsyncClasses();
	~VeritexAsyncClasses();
};
