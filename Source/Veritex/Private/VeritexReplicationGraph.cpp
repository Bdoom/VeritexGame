// Daniel Gleason (C) 2017
// 
// #include "VeritexReplicationGraph.h"
// #include "GameFramework/GameModeBase.h"
// #include "GameFramework/GameState.h"
// #include "GameFramework/PlayerState.h"
// #include "GameFramework/Pawn.h"
// #include "Engine/LevelScriptActor.h"
// #include "Net/UnrealNetwork.h"
// #include "Engine/LevelStreaming.h"
// #include "EngineUtils.h"
// #include "CoreGlobals.h"
// 
// #if WITH_GAMEPLAY_DEBUGGER
// #include "GameplayDebuggerCategoryReplicator.h"
// #endif
// 
// DEFINE_LOG_CATEGORY(LogVeritexReplicationGraph);
// 
// 
// float CVar_VeritexRepGraph_DestructionInfoMaxDist = 30000.f;
// static FAutoConsoleVariableRef CVarVeritexRepGraphDestructMaxDist(TEXT("VeritexRepGraph.DestructInfo.MaxDist"), CVar_VeritexRepGraph_DestructionInfoMaxDist, TEXT("Max distance (not squared) to rep destruct infos at"), ECVF_Default);
// 
// int32 CVar_VeritexRepGraph_DisplayClientLevelStreaming = 0;
// static FAutoConsoleVariableRef CVarVeritexRepGraphDisplayClientLevelStreaming(TEXT("VeritexRepGraph.DisplayClientLevelStreaming"), CVar_VeritexRepGraph_DisplayClientLevelStreaming, TEXT(""), ECVF_Default);
// 
// float CVar_VeritexRepGraph_CellSize = 10000.f;
// static FAutoConsoleVariableRef CVarVeritexRepGraphCellSize(TEXT("VeritexRepGraph.CellSize"), CVar_VeritexRepGraph_CellSize, TEXT(""), ECVF_Default);
// 
// Essentially "Min X" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
// float CVar_VeritexRepGraph_SpatialBiasX = -150000.f;
// static FAutoConsoleVariableRef CVarVeritexRepGraphSpatialBiasX(TEXT("VeritexRepGraph.SpatialBiasX"), CVar_VeritexRepGraph_SpatialBiasX, TEXT(""), ECVF_Default);
// 
// Essentially "Min Y" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
// float CVar_VeritexRepGraph_SpatialBiasY = -200000.f;
// static FAutoConsoleVariableRef CVarVeritexRepSpatialBiasY(TEXT("VeritexRepGraph.SpatialBiasY"), CVar_VeritexRepGraph_SpatialBiasY, TEXT(""), ECVF_Default);
// 
// How many buckets to spread dynamic, spatialized actors across. High number = more buckets = smaller effective replication frequency. This happens before individual actors do their own NetUpdateFrequency check.
// int32 CVar_VeritexRepGraph_DynamicActorFrequencyBuckets = 3;
// static FAutoConsoleVariableRef CVarVeritexRepDynamicActorFrequencyBuckets(TEXT("VeritexRepGraph.DynamicActorFrequencyBuckets"), CVar_VeritexRepGraph_DynamicActorFrequencyBuckets, TEXT(""), ECVF_Default);
// 
// int32 CVar_VeritexRepGraph_DisableSpatialRebuilds = 1;
// static FAutoConsoleVariableRef CVarVeritexRepDisableSpatialRebuilds(TEXT("VeritexRepGraph.DisableSpatialRebuilds"), CVar_VeritexRepGraph_DisableSpatialRebuilds, TEXT(""), ECVF_Default);
// 
// ----------------------------------------------------------------------------------------------------------
// 
// 
// 
// UVeritexReplicationGraph::UVeritexReplicationGraph()
// {
// 
// }
// 
// void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool bSpatialize, float ServerMaxTickRate)
// {
// 	AActor* CDO = Class->GetDefaultObject<AActor>();
// 	if (bSpatialize)
// 	{
// 		Info.CullDistanceSquared = CDO->NetCullDistanceSquared;
// 		UE_LOG(LogVeritexReplicationGraph, Log, TEXT("Setting cull distance for %s to %f (%f)"), *Class->GetName(), Info.CullDistanceSquared, FMath::Sqrt(Info.CullDistanceSquared));
// 	}
// 
// 	Info.ReplicationPeriodFrame = FMath::Max<uint32>((uint32)FMath::RoundToFloat(ServerMaxTickRate / CDO->NetUpdateFrequency), 1);
// 
// 	UClass* NativeClass = Class;
// 	while (!NativeClass->IsNative() && NativeClass->GetSuperClass() && NativeClass->GetSuperClass() != AActor::StaticClass())
// 	{
// 		NativeClass = NativeClass->GetSuperClass();
// 	}
// 
// 	UE_LOG(LogVeritexReplicationGraph, Log, TEXT("Setting replication period for %s (%s) to %d frames (%.2f)"), *Class->GetName(), *NativeClass->GetName(), Info.ReplicationPeriodFrame, CDO->NetUpdateFrequency);
// }
// 
// const UClass* GetParentNativeClass(const UClass* Class)
// {
// 	while (Class && !Class->IsNative())
// 	{
// 		Class = Class->GetSuperClass();
// 	}
// 
// 	return Class;
// }
// 
// void UVeritexReplicationGraph::ResetGameWorldState()
// {
// 	Super::ResetGameWorldState();
// 
// 	AlwaysRelevantStreamingLevelActors.Empty();
// 
// 	for (UNetReplicationGraphConnection* ConnManager : Connections)
// 	{
// 		for (UReplicationGraphNode* ConnectionNode : ConnManager->GetConnectionGraphNodes())
// 		{
// 			if (UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
// 			{
// 				AlwaysRelevantConnectionNode->ResetGameWorldState();
// 			}
// 		}
// 	}
// 
// 	for (UNetReplicationGraphConnection* ConnManager : PendingConnections)
// 	{
// 		for (UReplicationGraphNode* ConnectionNode : ConnManager->GetConnectionGraphNodes())
// 		{
// 			if (UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
// 			{
// 				AlwaysRelevantConnectionNode->ResetGameWorldState();
// 			}
// 		}
// 	}
// }
// 
// void UVeritexReplicationGraph::InitGlobalActorClassSettings()
// {
// 	Super::InitGlobalActorClassSettings();
// 
// 	// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 	// Programatically build the rules.
// 	// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
// 	auto AddInfo = [&](UClass* Class, EClassRepNodeMapping Mapping) { ClassRepNodePolicies.Set(Class, Mapping); };
// 	
// 	AddInfo(ALevelScriptActor::StaticClass(), EClassRepNodeMapping::NotRouted);				// Not needed
// 	AddInfo(APlayerState::StaticClass(), EClassRepNodeMapping::NotRouted);				// Special cased via UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter
// 	AddInfo(AReplicationGraphDebugActor::StaticClass(), EClassRepNodeMapping::NotRouted);				// Not needed. Replicated special case inside RepGraph
// 	AddInfo(AInfo::StaticClass(), EClassRepNodeMapping::RelevantAllConnections);	// Non spatialized, relevant to all
// 	AddInfo(AActor::StaticClass(), EClassRepNodeMapping::NotRouted);
// 	AddInfo(ANPC::StaticClass(), EClassRepNodeMapping::Spatialize_Dynamic);
// 	AddInfo(AStructure::StaticClass(), EClassRepNodeMapping::Spatialize_Static);
// 
// 	TArray<UClass*> AllReplicatedClasses;
// 
// 	for (TObjectIterator<UClass> It; It; ++It)
// 	{
// 		UClass* Class = *It;
// 		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
// 		if (!ActorCDO || !ActorCDO->GetIsReplicated())
// 		{
// 			continue;
// 		}
// 
// 		// Skip SKEL and REINST classes.
// 		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
// 		{
// 			continue;
// 		}
// 
// 		// --------------------------------------------------------------------
// 		// This is a replicated class. Save this off for the second pass below
// 		// --------------------------------------------------------------------
// 
// 		AllReplicatedClasses.Add(Class);
// 
// 		// Skip if already in the map (added explicitly)
// 		if (ClassRepNodePolicies.Contains(Class, false))
// 		{
// 			continue;
// 		}
// 
// 		auto ShouldSpatialize = [](const AActor* CDO)
// 		{
// 			return CDO->GetIsReplicated() && (!(CDO->bAlwaysRelevant || CDO->bOnlyRelevantToOwner || CDO->bNetUseOwnerRelevancy));
// 		};
// 
// 		auto GetLegacyDebugStr = [](const AActor* CDO)
// 		{
// 			return FString::Printf(TEXT("%s [%d/%d/%d]"), *CDO->GetClass()->GetName(), CDO->bAlwaysRelevant, CDO->bOnlyRelevantToOwner, CDO->bNetUseOwnerRelevancy);
// 		};
// 
// 		// Only handle this class if it differs from its super. There is no need to put every child class explicitly in the graph class mapping
// 		UClass* SuperClass = Class->GetSuperClass();
// 		if (AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
// 		{
// 			if (SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
// 				&& SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant
// 				&&	SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner
// 				&&	SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy
// 				)
// 			{
// 				continue;
// 			}
// 
// 			if (ShouldSpatialize(ActorCDO) == false && ShouldSpatialize(SuperCDO) == true)
// 			{
// 				UE_LOG(LogVeritexReplicationGraph, Log, TEXT("Adding %s to NonSpatializedChildClasses. (Parent: %s)"), *GetLegacyDebugStr(ActorCDO), *GetLegacyDebugStr(SuperCDO));
// 				NonSpatializedChildClasses.Add(Class);
// 			}
// 		}
// 
// 		if (ShouldSpatialize(ActorCDO))
// 		{
// 			AddInfo(Class, EClassRepNodeMapping::Spatialize_Dynamic);
// 		}
// 		else if (ActorCDO->bAlwaysRelevant && !ActorCDO->bOnlyRelevantToOwner)
// 		{
// 			AddInfo(Class, EClassRepNodeMapping::RelevantAllConnections);
// 		}
// 	}
// 
// 	// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 	// Setup FClassReplicationInfo. This is essentially the per class replication settings. Some we set explicitly, the rest we are setting via looking at the legacy settings on AActor.
// 	// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
// 	TArray<UClass*> ExplicitlySetClasses;
// 	auto SetClassInfo = [&](UClass* Class, const FClassReplicationInfo& Info) { GlobalActorReplicationInfoMap.SetClassInfo(Class, Info); ExplicitlySetClasses.Add(Class); };
// 
// 	FClassReplicationInfo PawnClassRepInfo;
// 	PawnClassRepInfo.DistancePriorityScale = 1.f;
// 	PawnClassRepInfo.StarvationPriorityScale = 1.f;
// 	PawnClassRepInfo.ActorChannelFrameTimeout = 4;
// 	PawnClassRepInfo.CullDistanceSquared = 15000.f * 15000.f; // Yuck
// 	SetClassInfo(APawn::StaticClass(), PawnClassRepInfo);
// 
// 	FClassReplicationInfo PlayerStateRepInfo;
// 	PlayerStateRepInfo.DistancePriorityScale = 0.f;
// 	PlayerStateRepInfo.ActorChannelFrameTimeout = 0;
// 	SetClassInfo(APlayerState::StaticClass(), PlayerStateRepInfo);
// 
// 	//UReplicationGraphNode_ActorListFrequencyBuckets::DefaultListSize = 12;	
// 
// 	// Set FClassReplicationInfo based on legacy settings from all replicated classes
// 	for (UClass* ReplicatedClass : AllReplicatedClasses)
// 	{
// 		if (ExplicitlySetClasses.FindByPredicate([&](const UClass* SetClass) { return ReplicatedClass->IsChildOf(SetClass); }) != nullptr)
// 		{
// 			continue;
// 		}
// 
// 		const bool bClassIsSpatialized = IsSpatialized(ClassRepNodePolicies.GetChecked(ReplicatedClass));
// 
// 		FClassReplicationInfo ClassInfo;
// 		InitClassReplicationInfo(ClassInfo, ReplicatedClass, bClassIsSpatialized, NetDriver->NetServerMaxTickRate);
// 		GlobalActorReplicationInfoMap.SetClassInfo(ReplicatedClass, ClassInfo);
// 	}
// 
// 
// 	// Print out what we came up with
// 	UE_LOG(LogVeritexReplicationGraph, Log, TEXT(""));
// 	UE_LOG(LogVeritexReplicationGraph, Log, TEXT("Class Routing Map: "));
// 	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EClassRepNodeMapping"));
// 	for (auto ClassMapIt = ClassRepNodePolicies.CreateIterator(); ClassMapIt; ++ClassMapIt)
// 	{
// 		const UClass* Class = CastChecked<UClass>(ClassMapIt.Key().ResolveObjectPtr());
// 		const EClassRepNodeMapping Mapping = ClassMapIt.Value();
// 
// 		// Only print if different than native class
// 		const UClass* ParentNativeClass = GetParentNativeClass(Class);
// 		const EClassRepNodeMapping* ParentMapping = ClassRepNodePolicies.Get(ParentNativeClass);
// 		if (ParentMapping && Class != ParentNativeClass && Mapping == *ParentMapping)
// 		{
// 			continue;
// 		}
// 
// 		UE_LOG(LogVeritexReplicationGraph, Log, TEXT("  %s (%s) -> %s"), *Class->GetName(), *GetNameSafe(ParentNativeClass), *Enum->GetNameStringByValue(static_cast<uint32>(Mapping)));
// 	}
// 
// 	UE_LOG(LogVeritexReplicationGraph, Log, TEXT(""));
// 	UE_LOG(LogVeritexReplicationGraph, Log, TEXT("Class Settings Map: "));
// 	FClassReplicationInfo DefaultValues;
// 	for (auto ClassRepInfoIt = GlobalActorReplicationInfoMap.CreateClassMapIterator(); ClassRepInfoIt; ++ClassRepInfoIt)
// 	{
// 		const UClass* Class = CastChecked<UClass>(ClassRepInfoIt.Key().ResolveObjectPtr());
// 		const FClassReplicationInfo& ClassInfo = ClassRepInfoIt.Value();
// 		UE_LOG(LogVeritexReplicationGraph, Log, TEXT("  %s (%s) -> %s"), *Class->GetName(), *GetNameSafe(GetParentNativeClass(Class)), *ClassInfo.BuildDebugStringDelta());
// 	}
// 
// 
// 	// Rep destruct infos based on CVar value
// 	DestructInfoMaxDistanceSquared = CVar_VeritexRepGraph_DestructionInfoMaxDist * CVar_VeritexRepGraph_DestructionInfoMaxDist;
// 
// 
// #if WITH_GAMEPLAY_DEBUGGER
// 	AGameplayDebuggerCategoryReplicator::NotifyDebuggerOwnerChange.AddUObject(this, &UVeritexReplicationGraph::OnGameplayDebuggerOwnerChange);
// #endif
// }
// 
// void UVeritexReplicationGraph::InitGlobalGraphNodes()
// {
// 	// Preallocate some replication lists.
// 	PreAllocateRepList(3, 12);
// 	PreAllocateRepList(6, 12);
// 	PreAllocateRepList(128, 64);
// 	PreAllocateRepList(512, 16);
// 
// 	// -----------------------------------------------
// 	//	Spatial Actors
// 	// -----------------------------------------------
// 
// 	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
// 	GridNode->CellSize = CVar_VeritexRepGraph_CellSize;
// 	GridNode->SpatialBias = FVector2D(CVar_VeritexRepGraph_SpatialBiasX, CVar_VeritexRepGraph_SpatialBiasY);
// 
// 	if (CVar_VeritexRepGraph_DisableSpatialRebuilds)
// 	{
// 		GridNode->AddSpatialRebuildBlacklistClass(AActor::StaticClass()); // Disable All spatial rebuilding
// 	}
// 
// 	AddGlobalGraphNode(GridNode);
// 
// 	// -----------------------------------------------
// 	//	Always Relevant (to everyone) Actors
// 	// -----------------------------------------------
// 	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
// 	AddGlobalGraphNode(AlwaysRelevantNode);
// 
// 	// -----------------------------------------------
// 	//	Player State specialization. This will return a rolling subset of the player states to replicate
// 	// -----------------------------------------------
// 	UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter* PlayerStateNode = CreateNewNode<UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter>();
// 	AddGlobalGraphNode(PlayerStateNode);
// }
// 
// void UVeritexReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
// {
// 	Super::InitConnectionGraphNodes(RepGraphConnection);
// 	
// 	UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = CreateNewNode<UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection>();
// 
// 	// This node needs to know when client levels go in and out of visibility
// 	RepGraphConnection->OnClientVisibleLevelNameAdd.AddUObject(AlwaysRelevantConnectionNode, &UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd);
// 	RepGraphConnection->OnClientVisibleLevelNameRemove.AddUObject(AlwaysRelevantConnectionNode, &UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove);
// 
// 	AddConnectionGraphNode(AlwaysRelevantConnectionNode, RepGraphConnection);
// }
// 
// #if WITH_GAMEPLAY_DEBUGGER
// void UVeritexReplicationGraph::OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner)
// {
// 	auto GetAlwaysRelevantForConnectionNode = [&](APlayerController* Controller) -> UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection*
// 	{
// 		if (OldOwner)
// 		{
// 			if (UNetConnection* NetConnection = OldOwner->GetNetConnection())
// 			{
// 				if (UNetReplicationGraphConnection* GraphConnection = FindOrAddConnectionManager(NetConnection))
// 				{
// 					for (UReplicationGraphNode* ConnectionNode : GraphConnection->GetConnectionGraphNodes())
// 					{
// 						if (UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
// 						{
// 							return AlwaysRelevantConnectionNode;
// 						}
// 					}
// 
// 				}
// 			}
// 		}
// 
// 		return nullptr;
// 	};
// 
// 	if (UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(OldOwner))
// 	{
// 		AlwaysRelevantConnectionNode->GameplayDebugger = nullptr;
// 	}
// 
// 	if (UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(Debugger->GetReplicationOwner()))
// 	{
// 		AlwaysRelevantConnectionNode->GameplayDebugger = Debugger;
// 	}
// }
// #endif
// 
// EClassRepNodeMapping UVeritexReplicationGraph::GetMappingPolicy(const UClass* Class)
// {
// 	EClassRepNodeMapping* PolicyPtr = ClassRepNodePolicies.Get(Class);
// 	EClassRepNodeMapping Policy = PolicyPtr ? *PolicyPtr : EClassRepNodeMapping::NotRouted;
// 	return Policy;
// }
// 
// void UVeritexReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
// {
// 	EClassRepNodeMapping Policy = GetMappingPolicy(ActorInfo.Class);
// 	switch (Policy)
// 	{
// 	case EClassRepNodeMapping::NotRouted:
// 	{
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::RelevantAllConnections:
// 	{
// 		if (ActorInfo.StreamingLevelName == NAME_None)
// 		{
// 			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
// 		}
// 		else
// 		{
// 			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindOrAdd(ActorInfo.StreamingLevelName);
// 			RepList.PrepareForWrite();
// 			RepList.ConditionalAdd(ActorInfo.Actor);
// 		}
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Static:
// 	{
// 		GridNode->AddActor_Static(ActorInfo, GlobalInfo);
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Dynamic:
// 	{
// 		GridNode->AddActor_Dynamic(ActorInfo, GlobalInfo);
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Dormancy:
// 	{
// 		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
// 		break;
// 	}
// 	};
// }
// 
// void UVeritexReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
// {
// 	EClassRepNodeMapping Policy = GetMappingPolicy(ActorInfo.Class);
// 	switch (Policy)
// 	{
// 	case EClassRepNodeMapping::NotRouted:
// 	{
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::RelevantAllConnections:
// 	{
// 		if (ActorInfo.StreamingLevelName == NAME_None)
// 		{
// 			AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
// 		}
// 		else
// 		{
// 			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindChecked(ActorInfo.StreamingLevelName);
// 			if (RepList.Remove(ActorInfo.Actor) == false)
// 			{
// 				UE_LOG(LogVeritexReplicationGraph, Warning, TEXT("Actor %s was not found in AlwaysRelevantStreamingLevelActors list. LevelName: %s"), *GetActorRepListTypeDebugString(ActorInfo.Actor), *ActorInfo.StreamingLevelName.ToString());
// 			}
// 		}
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Static:
// 	{
// 		GridNode->RemoveActor_Static(ActorInfo);
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Dynamic:
// 	{
// 		GridNode->RemoveActor_Dynamic(ActorInfo);
// 		break;
// 	}
// 
// 	case EClassRepNodeMapping::Spatialize_Dormancy:
// 	{
// 		GridNode->RemoveActor_Dormancy(ActorInfo);
// 		break;
// 	}
// 	};
// }
// 
// void UVeritexReplicationGraph::PrintRepNodePolicies()
// {
// 	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EClassRepNodeMapping"));
// 	if (!Enum)
// 	{
// 		return;
// 	}
// 
// 	GLog->Logf(TEXT("===================================="));
// 	GLog->Logf(TEXT("Veritex Replication Routing Policies"));
// 	GLog->Logf(TEXT("===================================="));
// 
// 	for (auto It = ClassRepNodePolicies.CreateIterator(); It; ++It)
// 	{
// 		FObjectKey ObjKey = It.Key();
// 
// 		EClassRepNodeMapping Mapping = It.Value();
// 
// 		GLog->Logf(TEXT("%-40s --> %s"), *GetNameSafe(ObjKey.ResolveObjectPtr()), *Enum->GetNameStringByValue(static_cast<uint32>(Mapping)));
// 	}
// }
// 
// void UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
// {
// 	QUICK_SCOPE_CYCLE_COUNTER(UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection_GatherActorListsForConnection);
// 
// 	UVeritexReplicationGraph* VeritexGraph = CastChecked<UVeritexReplicationGraph>(GetOuter());
// 
// 	ReplicationActorList.Reset();
// 
// 	ReplicationActorList.ConditionalAdd(Params.Viewer.InViewer);
// 	ReplicationActorList.ConditionalAdd(Params.Viewer.ViewTarget);
// 
// 	if (AVeritexPlayerController* PC = Cast<AVeritexPlayerController>(Params.Viewer.InViewer))
// 	{
// 		// 50% throttling of PlayerStates.
// 		const bool bReplicatePS = (Params.ConnectionManager.ConnectionId % 2) == (Params.ReplicationFrameNum % 2);
// 		if (bReplicatePS)
// 		{
// 			// Always return the player state to the owning player. Simulated proxy player states are handled by UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter
// 			if (APlayerState* PS = PC->PlayerState)
// 			{
// 				if (!bInitializedPlayerState)
// 				{
// 					bInitializedPlayerState = true;
// 					FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(PS);
// 					ConnectionActorInfo.ReplicationPeriodFrame = 1;
// 				}
// 
// 				ReplicationActorList.ConditionalAdd(PS);
// 			}
// 		}
// 
// 		if (AVeritexCharacter* Pawn = Cast<AVeritexCharacter>(PC->GetPawn()))
// 		{
// 			if (Pawn != LastPawn)
// 			{
// 				UE_LOG(LogVeritexReplicationGraph, Verbose, TEXT("Setting connection pawn cull distance to 0. %s"), *Pawn->GetName());
// 				LastPawn = Pawn;
// 				FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(Pawn);
// 				ConnectionActorInfo.CullDistanceSquared = 0.f;
// 			}
// 
// 			if (Pawn != Params.Viewer.ViewTarget)
// 			{
// 				ReplicationActorList.ConditionalAdd(Pawn);
// 			}
// 
// 		}
// 
// 		if (Params.Viewer.ViewTarget != LastPawn)
// 		{
// 			if (AVeritexCharacter* ViewTargetPawn = Cast<AVeritexCharacter>(Params.Viewer.ViewTarget))
// 			{
// 				UE_LOG(LogVeritexReplicationGraph, Verbose, TEXT("Setting connection view target pawn cull distance to 0. %s"), *ViewTargetPawn->GetName());
// 				LastPawn = ViewTargetPawn;
// 				FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(ViewTargetPawn);
// 				ConnectionActorInfo.CullDistanceSquared = 0.f;
// 			}
// 		}
// 	}
// 
// 	Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorList);
// 
// 	// Always relevant streaming level actors.
// 	FPerConnectionActorInfoMap& ConnectionActorInfoMap = Params.ConnectionManager.ActorInfoMap;
// 
// 	TMap<FName, FActorRepListRefView>& AlwaysRelevantStreamingLevelActors = VeritexGraph->AlwaysRelevantStreamingLevelActors;
// 
// 	for (int32 Idx = AlwaysRelevantStreamingLevelsNeedingReplication.Num() - 1; Idx >= 0; --Idx)
// 	{
// 		const FName& StreamingLevel = AlwaysRelevantStreamingLevelsNeedingReplication[Idx];
// 
// 		FActorRepListRefView* Ptr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel);
// 		if (Ptr == nullptr)
// 		{
// 			// No always relevant lists for that level
// 			UE_CLOG(CVar_VeritexRepGraph_DisplayClientLevelStreaming > 0, LogVeritexReplicationGraph, Display, TEXT("CLIENTSTREAMING Removing %s from AlwaysRelevantStreamingLevelActors because FActorRepListRefView is null. %s "), *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
// 			AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, 1, false);
// 			continue;
// 		}
// 
// 		FActorRepListRefView& RepList = *Ptr;
// 
// 		if (RepList.Num() > 0)
// 		{
// 			bool bAllDormant = true;
// 			for (FActorRepListType Actor : RepList)
// 			{
// 				FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);
// 				if (ConnectionActorInfo.bDormantOnConnection == false)
// 				{
// 					bAllDormant = false;
// 					break;
// 				}
// 			}
// 
// 			if (bAllDormant)
// 			{
// 				UE_CLOG(CVar_VeritexRepGraph_DisplayClientLevelStreaming > 0, LogVeritexReplicationGraph, Display, TEXT("CLIENTSTREAMING All AlwaysRelevant Actors Dormant on StreamingLevel %s for %s. Removing list."), *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
// 				AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, 1, false);
// 			}
// 			else
// 			{
// 				UE_CLOG(CVar_VeritexRepGraph_DisplayClientLevelStreaming > 0, LogVeritexReplicationGraph, Display, TEXT("CLIENTSTREAMING Adding always Actors on StreamingLevel %s for %s because it has at least one non dormant actor"), *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
// 				Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
// 			}
// 		}
// 		else
// 		{
// 			UE_LOG(LogVeritexReplicationGraph, Warning, TEXT("UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection - empty RepList %s"), *Params.ConnectionManager.GetName());
// 		}
// 
// 	}
// 
// #if WITH_GAMEPLAY_DEBUGGER
// 	if (GameplayDebugger)
// 	{
// 		ReplicationActorList.ConditionalAdd(GameplayDebugger);
// 	}
// #endif
// }
// 
// void UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
// {
// 	DebugInfo.Log(NodeName);
// 	DebugInfo.PushIndent();
// 	LogActorRepList(DebugInfo, NodeName, ReplicationActorList);
// 
// 	for (const FName& LevelName : AlwaysRelevantStreamingLevelsNeedingReplication)
// 	{
// 		UVeritexReplicationGraph* VeritexGraph = CastChecked<UVeritexReplicationGraph>(GetOuter());
// 		if (FActorRepListRefView* RepList = VeritexGraph->AlwaysRelevantStreamingLevelActors.Find(LevelName))
// 		{
// 			LogActorRepList(DebugInfo, FString::Printf(TEXT("AlwaysRelevant StreamingLevel List: %s"), *LevelName.ToString()), *RepList);
// 		}
// 	}
// 
// 	DebugInfo.PopIndent();
// }
// 
// void UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld)
// {
// 	UE_CLOG(CVar_VeritexRepGraph_DisplayClientLevelStreaming > 0, LogVeritexReplicationGraph, Display, TEXT("CLIENTSTREAMING ::OnClientLevelVisibilityAdd - %s"), *LevelName.ToString());
// 	AlwaysRelevantStreamingLevelsNeedingReplication.Add(LevelName);
// }
// 
// void UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove(FName LevelName)
// {
// 	UE_CLOG(CVar_VeritexRepGraph_DisplayClientLevelStreaming > 0, LogVeritexReplicationGraph, Display, TEXT("CLIENTSTREAMING ::OnClientLevelVisibilityRemove - %s"), *LevelName.ToString());
// 	AlwaysRelevantStreamingLevelsNeedingReplication.Remove(LevelName);
// }
// 
// void UVeritexReplicationGraphNode_AlwaysRelevant_ForConnection::ResetGameWorldState()
// {
// 	AlwaysRelevantStreamingLevelsNeedingReplication.Empty();
// }
// 
// UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter::UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter()
// {
// 	bRequiresPrepareForReplicationCall = true;
// }
// 
// void UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
// {
// 	const int32 ListIdx = Params.ReplicationFrameNum % ReplicationActorLists.Num();
// 	Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorLists[ListIdx]);
// 
// 	if (ForceNetUpdateReplicationActorList.Num() > 0)
// 	{
// 		Params.OutGatheredReplicationLists.AddReplicationActorList(ForceNetUpdateReplicationActorList);
// 	}
// }
// 
// void UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter::PrepareForReplication()
// {
// 	QUICK_SCOPE_CYCLE_COUNTER(UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter_GlobalPrepareForReplication);
// 
// 	ReplicationActorLists.Reset();
// 	ForceNetUpdateReplicationActorList.Reset();
// 
// 	ReplicationActorLists.AddDefaulted();
// 	FActorRepListRefView* CurrentList = &ReplicationActorLists[0];
// 	CurrentList->PrepareForWrite();
// 
// 	// We rebuild our lists of player states each frame. This is not as efficient as it could be but its the simplest way
// 	// to handle players disconnecting and keeping the lists compact. If the lists were persistent we would need to defrag them as players left.
// 
// 	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
// 	{
// 		APlayerState* PS = *It;
// 		if (IsActorValidForReplicationGather(PS) == false)
// 		{
// 			continue;
// 		}
// 
// 		if (CurrentList->Num() >= TargetActorsPerFrame)
// 		{
// 			ReplicationActorLists.AddDefaulted();
// 			CurrentList = &ReplicationActorLists.Last();
// 			CurrentList->PrepareForWrite();
// 		}
// 
// 		CurrentList->Add(PS);
// 	}
// }
// 
// void UVeritexReplicationGraphNode_PlayerStateFrequencyLimiter::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
// {
// 	DebugInfo.Log(NodeName);
// 	DebugInfo.PushIndent();
// 
// 	int32 i = 0;
// 	for (const FActorRepListRefView& List : ReplicationActorLists)
// 	{
// 		LogActorRepList(DebugInfo, FString::Printf(TEXT("Bucket[%d]"), i++), List);
// 	}
// 
// 	DebugInfo.PopIndent();
// }
