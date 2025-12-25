using UnrealBuildTool;
using System.Collections.Generic;

public class CatalystRisingTarget : TargetRules
{
	public CatalystRisingTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "CatalystRising" });
	}
}
