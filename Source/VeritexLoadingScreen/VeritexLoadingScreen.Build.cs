using UnrealBuildTool;
 
public class VeritexLoadingScreen : ModuleRules
{
	public VeritexLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", "Engine", "CoreUObject", "MoviePlayer", "Slate", "SlateCore", "InputCore", "UMG"
        });
 
		//The path for the header files
		PublicIncludePaths.AddRange(new string[] {"VeritexLoadingScreen/Public"});
 
		//The path for the source files
		PrivateIncludePaths.AddRange(new string[] {"VeritexLoadingScreen/Private"});
	}
}