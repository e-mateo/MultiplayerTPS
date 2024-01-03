// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ShooterMulti : ModuleRules
{
	public ShooterMulti(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "AIModule", "GameplayTasks", "GameplayCameras" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
