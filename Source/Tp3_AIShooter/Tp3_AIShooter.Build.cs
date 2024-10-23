// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tp3_AIShooter : ModuleRules
{
	public Tp3_AIShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
