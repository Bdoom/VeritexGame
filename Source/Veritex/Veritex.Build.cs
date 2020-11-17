// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Veritex : ModuleRules
{
    public Veritex(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils", "Foliage", "MoviePlayer", "AIModule", "ApexDestruction", "LoadingScreen", "GameplayTasks", "UWorksCore", "UWorks", "Json", "Http", "UMG", "Slate", "SlateCore", "NavigationSystem", "VictoryBPLibrary", "Voxel" });

//         PrivateDependencyModuleNames.AddRange(
//             new string[] {
//                 "ReplicationGraph"
//             }
//         );

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
            }
       );

        PrivateIncludePaths.AddRange(
            new string[]
            {
                "Veritex/Private"
            }
       );

        if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PrivateDependencyModuleNames.Add("GameplayDebugger");
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
        }

        bEnforceIWYU = true;

        PublicDefinitions.Add("AES_KEY=\"!z%C*F-JaNdRfUjXn2r5u8x/A?D(G+Kb\"");
    }
}
