using UnrealBuildTool;
using System.Collections.Generic;

public class VeritexEditorTarget : TargetRules
{
    public VeritexEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        BuildEnvironment = TargetBuildEnvironment.Shared;
        bPrecompile = true;

        //bUseUnityBuild = true;

        ExtraModuleNames.Add("Veritex");
    }
}

// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// 
// using UnrealBuildTool;
// using System.Collections.Generic;
// 
// public class VeritexEditorTarget : TargetRules
// {
// 	public VeritexEditorTarget(TargetInfo Target)
// 	{
// 		Type = TargetType.Editor;
// 
//    }
// 
//    //
//    // TargetRules interface.
//    //
// 
//    public override void SetupBinaries(
// 		TargetInfo Target,
// 		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
// 		ref List<string> OutExtraModuleNames
// 		)
// 	{
// 		OutExtraModuleNames.Add("Veritex");
// 	}
// }
