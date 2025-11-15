// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectSREditorTarget : TargetRules
{
	public ProjectSREditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6; // หรือ EngineIncludeOrderVersion.Latest
        CppStandard = CppStandardVersion.Cpp20;

        ExtraModuleNames.AddRange( new string[] { "ProjectSR" } );
	}
}
