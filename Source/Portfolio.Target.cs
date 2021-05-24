// Copyright Bruno Silva. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PortfolioTarget : TargetRules
{
	public PortfolioTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "Portfolio" } );
	}
}
