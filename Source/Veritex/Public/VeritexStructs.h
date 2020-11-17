
#pragma once
#include "VeritexStructs.generated.h"

USTRUCT(BlueprintType)
struct FVeritexFoliageDataSet
{
	GENERATED_BODY()

public:

	UPROPERTY()
		bool bInstanceEnabled = false;

	UPROPERTY()
		int32 Index;

	UPROPERTY()
		class UVeritexFoliageInstance* FoliageInstanceComponent;

	UPROPERTY()
		FTransform Transform;

};

USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = Item)
		FString ItemID;

	UPROPERTY(BlueprintReadWrite, Category = Item)
		int32 Count;

	UPROPERTY(BlueprintReadWrite, Category = Item)
		int32 Index;

	UPROPERTY(BlueprintReadWrite, Category = Item)
		FTransform ItemActorTransform;

};

inline FArchive& operator <<(FArchive& Ar, FItemData& Item)
{
	Ar << Item.ItemID;
	Ar << Item.Count;
	Ar << Item.Index;
	return Ar;
}

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = Character)
		FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		FTransform PlayerPosition;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		int32 Health;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		int32 MaxHealth;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MaxWeight;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MaxOxygen;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MovementSpeed;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MeleeDamage;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float Experience;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float LevelUpPoints;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		FString UniqueSteamID;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		FGuid UID;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float Food;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MaxFood;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float Water;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MaxWater;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		float MaxStamina;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		int32 Level;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		TArray<FItemData> Inventory;

	UPROPERTY(BlueprintReadWrite, Category = Character)
		TArray<FItemData> Equipment;

};

inline FArchive& operator<<(FArchive& Ar, FCharacterData& Character)
{
	Ar << Character.PlayerPosition;
	Ar << Character.Experience;
	Ar << Character.Health;
	Ar << Character.LevelUpPoints;
	Ar << Character.MaxHealth;
	Ar << Character.MaxOxygen;
	Ar << Character.MaxWeight;
	Ar << Character.Food;
	Ar << Character.MaxFood;
	Ar << Character.Water;
	Ar << Character.MaxWater;
	Ar << Character.MeleeDamage;
	Ar << Character.MovementSpeed;
	Ar << Character.UniqueSteamID;
	Ar << Character.UID;
	Ar << Character.PlayerName;
	Ar << Character.Level;
	Ar << Character.Inventory;
	Ar << Character.Equipment;
	return Ar;
}


USTRUCT(BlueprintType)
struct FStructureData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = Structure)
		FGuid UID;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		FString OwnedBySteamID;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		int32 Health;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		int32 MaxHealth;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		FTransform StructureLocation;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		FString StructureClassPath;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		int32 NeededTribeRank;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		TArray<FItemData> StorageItems;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		TArray<FName> StructureOwners;

	UPROPERTY(BlueprintReadWrite, Category = Structure)
		FColor StructureColor;

};

inline FArchive& operator<<(FArchive& Ar, FStructureData& Structure)
{
	Ar << Structure.UID;
	Ar << Structure.Health;
	Ar << Structure.MaxHealth;
	Ar << Structure.StructureLocation;
	Ar << Structure.StructureClassPath;
	Ar << Structure.NeededTribeRank;
	Ar << Structure.StorageItems;
	//Ar << Structure.OwnedBySteamID;
	return Ar;
}

USTRUCT(BlueprintType)
struct FNPCData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		FString ClassPath;

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		int32 Health;

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		int32 MaxHealth;

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		FTransform NPCTransform;

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		FString OwnedBy;

	UPROPERTY(BlueprintReadWrite, Category = "NPC Data")
		TArray<FStructureData> StructuresAttached;
};


USTRUCT(BlueprintType)
struct FWorldData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "World Data")
		TArray<FItemData> Items;

	UPROPERTY(BlueprintReadWrite, Category = "World Data")
		TArray<FStructureData> Structures;

	UPROPERTY(BlueprintReadWrite, Category = "World Data")
		TArray<FNPCData> NPCs;

};

inline FArchive& operator<<(FArchive& Ar, FWorldData& WorldData)
{
	Ar << WorldData.Items;
	Ar << WorldData.Structures;
	return Ar;
}

inline FArchive& operator <<(FArchive& Ar, FNPCData& NPC)
{
	return Ar;
}

UENUM(BlueprintType)
enum class EStructureType : uint8
{
	Foundation,
	Wall,
	Ceiling,
	Door,
	DoorFrame,
	Storage_Small,
	Storage_Medium,
	Storage_Large,
	StairCase,
	Bed,
	Turret,
	CampFire,
	Furnace,
	PartyStone,
	FoodPile,
	WoodPile,
	StonePile

};

UENUM(BlueprintType)
enum class EStructureQuality : uint8
{
	Thatch,
	Wood,
	Stone,
	Metal
};

UENUM(BlueprintType)
enum class EWeatherMode : uint8
{
	Rain,
	Snow,
	ClearSkys
};

USTRUCT(BlueprintType)
struct FPlayerSkillData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Player Stat Data")
		float WoodCuttingLevel;

	UPROPERTY(BlueprintReadWrite, Category = "Player Stat Data")
		float WoodCuttingExperience;
	
};