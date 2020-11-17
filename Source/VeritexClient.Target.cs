
// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class VeritexClientTarget : TargetRules
{

    public VeritexClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        BuildEnvironment = TargetBuildEnvironment.Shared;
        ExtraModuleNames.Add("Veritex");
    }
    
}


//// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

//using UnrealBuildTool;
//using System.Collections.Generic;

//public class VeritexClientTarget : TargetRules
//{

//    public VeritexClientTarget(TargetInfo Target)
//	{
//		Type = TargetType.Client;
//        	bOutputToEngineBinaries = true;
//	}

//	//
//	// TargetRules interface.
//	//
//	public override void SetupBinaries(
//		TargetInfo Target,
//		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
//		ref List<string> OutExtraModuleNames
//		)
//	{
//		base.SetupBinaries(Target, ref OutBuildBinaryConfigurations, ref OutExtraModuleNames);
//		OutExtraModuleNames.Add("Veritex");
//	}

//	public override void SetupGlobalEnvironment(
//        TargetInfo Target,
//        ref LinkEnvironmentConfiguration OutLinkEnvironmentConfiguration,
//        ref CPPEnvironmentConfiguration OutCPPEnvironmentConfiguration
//        )
//    {
//    }

//	public override bool ShouldUseSharedBuildEnvironment(TargetInfo Target)
//	{
//		return true;
//	}
//}
