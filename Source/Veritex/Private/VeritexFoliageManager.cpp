// Daniel Gleason (C) 2017

#include "VeritexFoliageManager.h"
#include "InstancedFoliageActor.h"
#include "VeritexFoliageInstance.h"
#include "VeritexStructs.h"
#include "EngineUtils.h"

DECLARE_STATS_GROUP(TEXT("VeritexFoliage"), STATGROUP_VeritexFoliage, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Foliage Update"), STAT_FoliageUpdate, STATGROUP_VeritexFoliage);
DECLARE_CYCLE_STAT(TEXT("Finished Foliage Update"), STAT_FoliageUpdateFinished, STATGROUP_VeritexFoliage);
DECLARE_CYCLE_STAT(TEXT("Enable Collision Update"), STAT_EnableCollision, STATGROUP_VeritexFoliage);
DECLARE_CYCLE_STAT(TEXT("Disable Collision Update"), STAT_DisableCollision, STATGROUP_VeritexFoliage);
DECLARE_CYCLE_STAT(TEXT("Determine Active Grids"), STAT_FoliageDetermineActiveGrids, STATGROUP_VeritexFoliage);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Active Grids"), STAT_FoliageActiveGrids, STATGROUP_VeritexFoliage);

// Sets default values
AVeritexFoliageManager::AVeritexFoliageManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	PrimaryActorTick.TickInterval = 1.f;

	bReplicates = true;
	bAlwaysRelevant = true;

	RemainingUpdateTime = 0.f;

	CurrentFoliageActorIndex = 0;
	CurrentFoliageComponentIndex = 0;
	CurrentFoliageInstanceIndex = 0;
}

bool AVeritexFoliageManager::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	return Super::ProcessConsoleExec(Cmd, Ar, Executor);
}

// Called when the game starts or when spawned
void AVeritexFoliageManager::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AInstancedFoliageActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AInstancedFoliageActor* FoliageActor = *ActorItr;
		TArray<class UVeritexFoliageInstance*> FoliageComps;
		FoliageActor->GetComponents<UVeritexFoliageInstance>(FoliageComps);
		for (UVeritexFoliageInstance* Comp : FoliageComps)
		{
			for (int i = 0; i < Comp->GetNumInstances(); ++i)
			{
				FVeritexFoliageDataSet FoliageInfo;
				FoliageInfo.FoliageInstanceComponent = Comp;
				FoliageInfo.Index = i;
				FTransform InstanceTransform;
				Comp->GetInstanceTransform_public(i, InstanceTransform);
				FoliageInfo.Transform = InstanceTransform;
				FoliageData.Add(FoliageInfo);
			}
		}
	}

	//CreateFoliageGrids();
}

void AVeritexFoliageManager::CreateFoliageGrids()
{

	for (TActorIterator<AInstancedFoliageActor> It(GetWorld()); It; ++It)
	{
		class AInstancedFoliageActor* FoliageActor = *It;

		TArray<class UVeritexFoliageInstance*> FoliageComponents;
		FoliageActor->GetComponents<UVeritexFoliageInstance>(FoliageComponents);

		for (UVeritexFoliageInstance* FoliageComponent : FoliageComponents)
		{
			int32 InstanceCount = FoliageComponent->GetInstanceCount();
			for (int32 Idx = 0; Idx < InstanceCount; ++Idx)
			{
				FTransform InstanceTransform;
				const bool bResult = FoliageComponent->GetInstanceTransform(Idx, InstanceTransform, true);
				check(bResult);

				int32 GridX = FMath::TruncToInt(InstanceTransform.GetLocation().X / VERITEX_FOLIAGE_GRID_SIZE) + VERITEX_FOLIAGE_INDEX_OFFSET;
				int32 GridY = FMath::TruncToInt(InstanceTransform.GetLocation().Y / VERITEX_FOLIAGE_GRID_SIZE) + VERITEX_FOLIAGE_INDEX_OFFSET;

				check(GridX >= 0);
				check(GridY >= 0);

				if (GridX >= FoliageGrids.Num())
				{
					FoliageGrids.AddDefaulted(GridX - FoliageGrids.Num() + 1);
				}

				if (GridY >= FoliageGrids[GridX].Num())
				{
					FoliageGrids[GridX].AddDefaulted(GridY - FoliageGrids[GridX].Num() + 1);
				}

				FoliageGrids[GridX][GridY].Add(FVeritexFoliageGridInstanceInfo(FoliageComponent, Idx));
			}
		}
	}

	for (TActorIterator<AInstancedFoliageActor> It(GetWorld()); It; ++It)
	{
		class AInstancedFoliageActor* IFA = *It;
		FoliageActors.Add(IFA);
	}

	// Cache components for initial foliage actor.
	if (FoliageActors.Num() > 0)
	{
		CurrentFoliageComponents.Empty();
		FoliageActors[0]->GetComponents<UVeritexFoliageInstance>(CurrentFoliageComponents);
	}

	CurrentFoliageActorIndex = 0;
	CurrentFoliageComponentIndex = 0;
	CurrentFoliageInstanceIndex = 0;

	RemainingUpdateTime = 1.f;

}

// Called every frame
void AVeritexFoliageManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SCOPE_CYCLE_COUNTER(STAT_FoliageUpdate);

	PlayerLocations.Empty();

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		if (It && (*It)->IsPlayerControlled())
		{
			PlayerLocations.Add((*It)->GetActorLocation());
		}

	}

	for (FVeritexFoliageDataSet& DataSet : FoliageData)
	{
		for (FVector PlayerLocation : PlayerLocations)
		{
			if (DataSet.FoliageInstanceComponent)
			{
				if (FVector::DistSquared(DataSet.Transform.GetLocation(), PlayerLocation) < 35000 * 35000)
				{
					SCOPE_CYCLE_COUNTER(STAT_EnableCollision);
					if (!DataSet.bInstanceEnabled)
					{
						DataSet.FoliageInstanceComponent->EnableInstanceCollision(DataSet.Index);
						DataSet.bInstanceEnabled = true;
					}
				}
				else
				{
					SCOPE_CYCLE_COUNTER(STAT_DisableCollision);
					if (DataSet.bInstanceEnabled)
					{
						DataSet.FoliageInstanceComponent->DisableInstanceCollision(DataSet.Index);
						DataSet.bInstanceEnabled = false;
					}
				}
			}
		}
	}

	SCOPE_CYCLE_COUNTER(STAT_FoliageUpdateFinished);


	// 	{
	// 		SCOPE_CYCLE_COUNTER(STAT_FoliageDetermineActiveGrids);
	// 
	// 		TArray<FIntPoint> NewGrids;
	// 
	// 		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	// 		{
	// 			if (It && (*It)->IsPlayerControlled())
	// 			{
	// 				DetermineGrids((*It)->GetActorLocation(), NewGrids);
	// 			}
	// 		}
	// 
	// 		// remove old grids.
	// 		for (int32 Idx = 0; Idx < ActiveGrids.Num(); ++Idx)
	// 		{
	// 			if (!NewGrids.Contains(FIntPoint(ActiveGrids[Idx].X, ActiveGrids[Idx].Y)))
	// 			{
	// 				ActiveGrids[Idx].bPendingRemoval = true;
	// 			}
	// 		}
	// 
	// 		// add new grids.
	// 		for (int32 Idx = 0; Idx < NewGrids.Num(); ++Idx)
	// 		{
	// 			AddActiveGrid(NewGrids[Idx].X, NewGrids[Idx].Y);
	// 		}
	// 	}
	// 
	// 	SET_FLOAT_STAT(STAT_FoliageActiveGrids, (float)ActiveGrids.Num());
	// 
	// 	for (int32 Idx = 0; Idx < ActiveGrids.Num(); ++Idx)
	// 	{
	// 		FVeritexFoliageActiveGrid& ActiveGrid = ActiveGrids[Idx];
	// 
	// 		if (ActiveGrid.bPendingRemoval)
	// 		{
	// 			if (ActiveGrid.ActivatedInstances.Num() > 0)
	// 			{
	// 				FVeritexFoliageGridInstanceInfo& Info = ActiveGrid.ActivatedInstances.Last();
	// 				Info.DisableCollision();
	// 				ActiveGrid.ActivatedInstances.RemoveAt(ActiveGrid.ActivatedInstances.Num() - 1);
	// 			}
	// 			else
	// 			{
	// 				ActiveGrids.RemoveAt(Idx);
	// 				--Idx;
	// 			}
	// 		}
	// 	}
	// 
	// 	if (Role == ROLE_Authority)
	// 	{
	// 		RemainingUpdateTime -= DeltaTime;
	// 		if (RemainingUpdateTime <= 0.f)
	// 		{
	// 			RemainingUpdateTime = 0.f;
	// 
	// 			for (int32 Index = 0; Index < 100; ++Index)
	// 			{
	// 				PrepareNextStep();
	// 			}
	// 		}
	// 	}
	// 
	// 	GLog->Log("ActiveGrids: " + FString::FromInt(ActiveGrids.Num()));
	// 	GLog->Log("FoliageGrids(how many grids): " + FString::FromInt(FoliageGrids.Num()));
}

void AVeritexFoliageManager::PrepareNextStep()
{
	bool bFoliageActorChanged = false;

	++CurrentFoliageInstanceIndex;

	class UVeritexFoliageInstance* const CurrentFoliageComponent = CurrentFoliageComponents.IsValidIndex(CurrentFoliageComponentIndex) ? CurrentFoliageComponents[CurrentFoliageComponentIndex] : nullptr;
	if (!CurrentFoliageComponent || CurrentFoliageInstanceIndex >= CurrentFoliageComponent->GetInstanceCount())
	{
		// We reached to end of the instances. Go next component.
		++CurrentFoliageComponentIndex;
		CurrentFoliageInstanceIndex = 0;
	}

	if (CurrentFoliageComponentIndex >= CurrentFoliageComponents.Num())
	{
		// We reached to end of components. Go next foliage actor.
		++CurrentFoliageActorIndex;
		CurrentFoliageComponentIndex = 0;

		bFoliageActorChanged = true;
	}

	if (CurrentFoliageActorIndex >= FoliageActors.Num())
	{
		// We reached to end of foliage actors. Go first foliage actor.
		CurrentFoliageActorIndex = 0;

		bFoliageActorChanged = true;
	}

	if (bFoliageActorChanged)
	{
		// Cache components for foliage actor.
		CurrentFoliageComponents.Empty();
		if (FoliageActors.IsValidIndex(CurrentFoliageActorIndex))
		{
			FoliageActors[CurrentFoliageActorIndex]->GetComponents<UVeritexFoliageInstance>(CurrentFoliageComponents);
		}
	}
}


static void VeritexConvertLocationToFoliageGrid(const FVector& Location, int32& GridX, int32& GridY)
{
	GridX = FMath::TruncToInt(Location.X / VERITEX_FOLIAGE_GRID_SIZE) + VERITEX_FOLIAGE_INDEX_OFFSET;
	GridY = FMath::TruncToInt(Location.Y / VERITEX_FOLIAGE_GRID_SIZE) + VERITEX_FOLIAGE_INDEX_OFFSET;

	check(GridX >= 0);
	check(GridY >= 0);
}

void AVeritexFoliageManager::DetermineGrids(const FVector& Location, TArray<FIntPoint>& Grids) const
{
	int32 GridX;
	int32 GridY;
	VeritexConvertLocationToFoliageGrid(Location, GridX, GridY);

	Grids.Add(FIntPoint(GridX + 0, GridY + 0));	// Center

	Grids.Add(FIntPoint(GridX + 1, GridY - 1));	// Front Left
	Grids.Add(FIntPoint(GridX + 1, GridY));		// Front Center
	Grids.Add(FIntPoint(GridX + 1, GridY + 1));	// Front Right

	Grids.Add(FIntPoint(GridX + 0, GridY - 1));	// Left Center
	Grids.Add(FIntPoint(GridX + 0, GridY + 1));	// Right Center

	Grids.Add(FIntPoint(GridX - 1, GridY - 1));	// Back Left
	Grids.Add(FIntPoint(GridX - 1, GridY));		// Back Center
	Grids.Add(FIntPoint(GridX - 1, GridY + 1));	// Back Right
}


bool AVeritexFoliageManager::IsGridActive(int32 X, int32 Y) const
{
	for (int32 Idx = 0; Idx < ActiveGrids.Num(); ++Idx)
	{
		if (ActiveGrids[Idx].X == X && ActiveGrids[Idx].Y == Y)
		{
			return true;
		}
	}

	return false;
}

void AVeritexFoliageManager::AddActiveGrid(int32 X, int32 Y)
{
	if (FoliageGrids.IsValidIndex(X))
	{
		if (FoliageGrids[X].IsValidIndex(Y))
		{
			for (int32 Idx = 0; Idx < ActiveGrids.Num(); ++Idx)
			{
				if (ActiveGrids[Idx].X == X && ActiveGrids[Idx].Y == Y)
				{
					FVeritexFoliageActiveGrid& ActiveGrid = ActiveGrids[Idx];

					if (ActiveGrid.bPendingRemoval)
					{
						// Grid already active, just remove pending removal flag.
						ActiveGrid.bPendingRemoval = false;

						TArray<FVeritexFoliageGridInstanceInfo>& GridInstances = FoliageGrids[X][Y];
						for (int32 Idx2 = 0; Idx2 < GridInstances.Num(); ++Idx2)
						{
							if (!ActiveGrid.ActivatedInstances.Contains(GridInstances[Idx2]))
							{
								GridInstances[Idx2].EnableCollision();
								ActiveGrid.ActivatedInstances.Add(GridInstances[Idx2]);
							}
						}
					}

					return;
				}
			}

			// Add new entry for grid.
			FVeritexFoliageActiveGrid ActiveGrid(X, Y);
			TArray<FVeritexFoliageGridInstanceInfo>& GridInstances = FoliageGrids[ActiveGrid.X][ActiveGrid.Y];
			for (int32 Idx2 = 1; Idx2 < GridInstances.Num(); ++Idx2)
			{
				if (!ActiveGrid.ActivatedInstances.Contains(GridInstances[Idx2]))
				{
					GridInstances[Idx2].EnableCollision();
					ActiveGrid.ActivatedInstances.Add(GridInstances[Idx2]);
				}
			}

			ActiveGrids.Add(ActiveGrid);
		}
	}
}

void AVeritexFoliageManager::RemoveActiveGrid(int32 X, int32 Y, bool bForce /*= false*/)
{
	for (int32 Idx = 0; Idx < ActiveGrids.Num(); ++Idx)
	{
		if (ActiveGrids[Idx].X == X && ActiveGrids[Idx].Y == Y)
		{
			ActiveGrids[Idx].bPendingRemoval = true;

			if (bForce)
			{
				for (FVeritexFoliageGridInstanceInfo& It : ActiveGrids[Idx].ActivatedInstances)
				{
					It.DisableCollision();
				}
				ActiveGrids[Idx].ActivatedInstances.Empty();
			}

			return;
		}
	}
}

FVeritexScopedFoliageCollisionActivator::FVeritexScopedFoliageCollisionActivator(class UWorld* InWorld, FVector InLocation)
	: FoliageManager(nullptr)
{
	for (TActorIterator<AVeritexFoliageManager> It(InWorld); It; ++It)
	{
		FoliageManager = *It;
	}

	if (FoliageManager)
	{
		FoliageManager->DetermineGrids(InLocation, ActiveGrids);

		for (const FIntPoint& It : ActiveGrids)
		{
			FoliageManager->AddActiveGrid(It.X, It.Y);
		}
	}
}

FVeritexScopedFoliageCollisionActivator::~FVeritexScopedFoliageCollisionActivator()
{
	if (FoliageManager)
	{
		for (const FIntPoint& It : ActiveGrids)
		{
			FoliageManager->RemoveActiveGrid(It.X, It.Y, true);
		}
	}
}

FVeritexFoliageGridInstanceInfo::FVeritexFoliageGridInstanceInfo()
	: Index(INDEX_NONE)
{

}

FVeritexFoliageGridInstanceInfo::FVeritexFoliageGridInstanceInfo(class UVeritexFoliageInstance* InComponent, int32 InIndex)
	: Component(InComponent)
	, Index(InIndex)
{
}

FTransform FVeritexFoliageGridInstanceInfo::GetTransform() const
{
	if (IsValid())
	{
		FTransform Transform;
		Component->GetInstanceTransform(Index, Transform, true);
		return Transform;
	}

	return FTransform::Identity;
}

void FVeritexFoliageGridInstanceInfo::EnableCollision()
{
	if (IsValid())
	{
		Component->EnableInstanceCollision(Index);
	}
}

void FVeritexFoliageGridInstanceInfo::DisableCollision()
{
	if (IsValid())
	{
		Component->DisableInstanceCollision(Index);
	}
}

FVeritexFoliageActiveGrid::FVeritexFoliageActiveGrid(int32 InX, int32 InY)
	: X(InX)
	, Y(InY)
	, bPendingRemoval(false)
{

}