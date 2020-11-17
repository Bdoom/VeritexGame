// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class VeritexTarget : TargetRules
{
	public VeritexTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        //bUsesSteam = true;
        bUseLoggingInShipping = true;

        ExtraModuleNames.Add("Veritex");
    }
}
