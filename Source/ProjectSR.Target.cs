// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectSRTarget : TargetRules
{
	public ProjectSRTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6; // หรือ EngineIncludeOrderVersion.Latest
        CppStandard = CppStandardVersion.Cpp20;

        ExtraModuleNames.AddRange( new string[] { "ProjectSR" } );
	}
}
