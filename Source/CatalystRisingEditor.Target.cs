using UnrealBuildTool;
using System.Collections.Generic;

public class CatalystRisingEditorTarget : TargetRules
{
	public CatalystRisingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "CatalystRising" });
	}
}
