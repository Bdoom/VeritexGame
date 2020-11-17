// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VeritexFoliageManager.generated.h"


struct FVeritexFoliageDataSet;

#define VERITEX_FOLIAGE_GRID_SIZE 2000
#define VERITEX_FOLIAGE_INDEX_OFFSET 35535

struct FVeritexFoliageGridInstanceInfo
{

	UPROPERTY()
		class UVeritexFoliageInstance* Component;

	UPROPERTY()
		int32 Index;

	FVeritexFoliageGridInstanceInfo();

	FVeritexFoliageGridInstanceInfo(class UVeritexFoliageInstance* InComponent, int32 InIndex);

	bool IsValid()const
	{
		TWeakObjectPtr<UVeritexFoliageInstance> WeakComp = Component;
		return (WeakComp.IsValid() && Component != nullptr && Index != INDEX_NONE);
	}

	bool operator==(const FVeritexFoliageGridInstanceInfo& rhs)const
	{
		return Component == rhs.Component && Index == rhs.Index;
	}

	FTransform GetTransform() const;
	void EnableCollision();
	void DisableCollision();
};

struct FVeritexFoliageActiveGrid
{

	UPROPERTY()
		int32 X;

	UPROPERTY()
		int32 Y;

	UPROPERTY()
		TArray<FVeritexFoliageGridInstanceInfo> ActivatedInstances;

	UPROPERTY()
		bool bPendingRemoval;

	FVeritexFoliageActiveGrid(int32 InX, int32 InY);

	bool operator==(const FVeritexFoliageActiveGrid& RHS)const
	{
		return RHS.X == X && RHS.Y == Y;
	}
};

struct FVeritexScopedFoliageCollisionActivator
{
	UPROPERTY()
		class AVeritexFoliageManager* FoliageManager;

	UPROPERTY()
		TArray<FIntPoint> ActiveGrids;

	FVeritexScopedFoliageCollisionActivator(class UWorld* InWorld, FVector InLocation);
	~FVeritexScopedFoliageCollisionActivator();
};

UCLASS()
class VERITEX_API AVeritexFoliageManager : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVeritexFoliageManager();

	UFUNCTION()
		void CreateFoliageGrids();

	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;

	UPROPERTY()
		TArray<FVeritexFoliageDataSet> FoliageData;

	UPROPERTY()
		TArray<FVector> PlayerLocations;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// 3-dimensional array. GridX|GridY|Instances
	TArray<TArray<TArray<FVeritexFoliageGridInstanceInfo>>> FoliageGrids;

	TArray<FVeritexFoliageActiveGrid> ActiveGrids;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PrepareNextStep();
	void DetermineGrids(const FVector& Location, TArray<FIntPoint>& Grids)const;

	bool IsGridActive(int32 X, int32 Y)const;
	void AddActiveGrid(int32 X, int32 Y);
	void RemoveActiveGrid(int32 X, int32 Y, bool bForce = false);

private:
	UPROPERTY(Transient)
		TArray<class AInstancedFoliageActor*> FoliageActors;

	int32 CurrentFoliageActorIndex;

	UPROPERTY(Transient)
		TArray<class UVeritexFoliageInstance*> CurrentFoliageComponents;

	int32 CurrentFoliageComponentIndex;
	int32 CurrentFoliageInstanceIndex;

	float RemainingUpdateTime;

};
