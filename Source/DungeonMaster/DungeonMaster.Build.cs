// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DungeonMaster : ModuleRules
{
	public DungeonMaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"DungeonMaster",
			"DungeonMaster/Variant_Strategy",
			"DungeonMaster/Variant_Strategy/UI",
			"DungeonMaster/Variant_TwinStick",
			"DungeonMaster/Variant_TwinStick/AI",
			"DungeonMaster/Variant_TwinStick/Gameplay",
			"DungeonMaster/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
