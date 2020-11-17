// Daniel Gleason (C) 2017

#include "VeritexAsyncClasses.h"
#include "Veritex.h"

VeritexAsyncClasses::VeritexAsyncClasses()
{
}

VeritexAsyncClasses::~VeritexAsyncClasses()
{
}

void FSaveLoadWorldTask::LoadCharacterFromUniqueSteamID()
{
	FCharacterData CharData;
	bool bKeepAlive_L = bKeepAlive;
	FVector SpawnLocation_L = SpawnLocation;

	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *(UVeritexFunctionLibrary::PathToCharacters() + UniqueSteamID + FString(".json"))))
	{
		FJsonObjectConverter::JsonObjectStringToUStruct<FCharacterData>(JsonString, &CharData, 0, 0);
	}

	AsyncTask(ENamedThreads::GameThread, [CharData, bKeepAlive_L, SpawnLocation_L]()
	{
		//AVeritexGameMode::OnCharacterDataLoaded_Delegate.Execute(CharData, bKeepAlive_L, SpawnLocation_L);
	});
}

void FSaveLoadWorldTask::SaveCharacterDataToFile()
{
	FString OutputJsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FCharacterData::StaticStruct(), &CharacterData, OutputJsonString, 0, 0);
	FString EncryptedString = "";
	EncryptedString = UVeritexFunctionLibrary::Encrypt(OutputJsonString);

	if (EncryptedString.IsEmpty())
	{
		return;
	}


	if (!IsCreativeMode)
	{
		FFileHelper::SaveStringToFile(EncryptedString, *(UVeritexFunctionLibrary::PathToCharacters() + UniqueSteamID + FString(".json")));
	}
	else
	{
		FFileHelper::SaveStringToFile(EncryptedString, *(UVeritexFunctionLibrary::PathToCharacters() + UniqueSteamID + FString("_Creative") + FString(".json")));
	}
}

void FSaveLoadWorldTask::LoadFromFile()
{

	if (!UniqueSteamID.IsEmpty() && !UniqueSteamID.Equals(" "))
	{
		LoadCharacterFromUniqueSteamID();
		return;
	}

	FWorldData WorldDataLocal = WorldData;

	TArray<uint8> BinaryArray;
	bool bLoaded = FFileHelper::LoadFileToArray(BinaryArray, *UVeritexFunctionLibrary::PathToUniverseData());

	if (bLoaded)
	{
		if (BinaryArray.Num() <= 0)
		{
			return;
		}

		FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
		FromBinary.Seek(0);
		SaveLoadData(FromBinary, WorldDataLocal);

		AsyncTask(ENamedThreads::GameThread, [WorldDataLocal]()
		{
			AVeritexGameMode::WorldDataLoaded.Execute(WorldDataLocal);
		});

		FromBinary.FlushCache();
		BinaryArray.Empty();
		FromBinary.Close();

	}
}

void FSaveLoadWorldTask::SaveToFile()
{
	if (!UniqueSteamID.IsEmpty() && !UniqueSteamID.Equals(" "))
	{
		SaveCharacterDataToFile();
		return;
	}

	FString OutputJsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FWorldData::StaticStruct(), &WorldData, OutputJsonString, 0, 0);
	OutputJsonString = UVeritexFunctionLibrary::Encrypt(OutputJsonString);

	FFileHelper::SaveStringToFile(OutputJsonString, *(UVeritexFunctionLibrary::PathToVeritexServerData() + FString("Universe.json")));
}

void FSaveLoadWorldTask::DoWork()
{
	if (bSaving)
	{
		SaveToFile();
	}
	else
	{
		LoadFromFile();
	}
}
