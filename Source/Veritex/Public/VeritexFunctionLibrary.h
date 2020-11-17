// Daniel Gleason (C) 2017

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "HAL/PlatformFilemanager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/LocalTimestampDirectoryVisitor.h"
#include "Engine/LocalPlayer.h"
#include "InstancedFoliageActor.h"
#include "VeritexWorldSettings.h"

#include "VeritexFunctionLibrary.generated.h"


UCLASS()
class VERITEX_API UVeritexFunctionLibrary : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AES Encryption")
		static FString Encrypt(FString StringToEncrypt);

	UFUNCTION(BlueprintCallable, Category = "AES Encryption")
		static FString Decrypt(FString StringToEncrypt);

	UFUNCTION(BlueprintPure, Category = ActorComponent)
		static FORCEINLINE bool IsComponentPendingKill(UActorComponent* InComponent)
	{
		if (InComponent != NULL && InComponent != nullptr)
		{
			return InComponent->IsPendingKill();
		}
		else
		{
			return true;
		}
	}

	UFUNCTION(BlueprintPure, Category = Actor)
		static FORCEINLINE bool IsActorPendingKill(AActor* TheActor)
	{
		if (TheActor != NULL && TheActor != nullptr)
		{
			return TheActor->IsPendingKill();
		}
		else
		{
			return true;
		}

	}

	UFUNCTION(BlueprintPure, Category = Widgets)
		static FORCEINLINE bool ProjectWorldToScreenBidirectional(APlayerController const* Player, const FVector& WorldPosition, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative)
	{
		FVector Projected;
		bool bSuccess = false;

		ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
		if (LP && LP->ViewportClient)
		{
			// get the projection data
			FSceneViewProjectionData ProjectionData;
			if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData))
			{
				const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
				const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();

				FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
				if (Result.W < 0.f) { bTargetBehindCamera = true; }
				if (Result.W == 0.f) { Result.W = 1.f; } // Prevent Divide By Zero

				const float RHW = 1.f / FMath::Abs(Result.W);
				Projected = FVector(Result.X, Result.Y, Result.Z) * RHW;

				// Normalize to 0..1 UI Space
				const float NormX = (Projected.X / 2.f) + 0.5f;
				const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

				Projected.X = (float)ViewRectangle.Min.X + (NormX * (float)ViewRectangle.Width());
				Projected.Y = (float)ViewRectangle.Min.Y + (NormY * (float)ViewRectangle.Height());

				bSuccess = true;
				ScreenPosition = FVector2D(Projected.X, Projected.Y);

				if (bPlayerViewportRelative)
				{
					ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
				}
			}
			else
			{
				ScreenPosition = FVector2D(1234, 5678);
			}
		}

		return bSuccess;
	}

	static FORCEINLINE FString PathToDatabase()
	{
		return FPaths::ProjectSavedDir() + "Veritex_Server_Data/Veritex.db";
	}

	static FORCEINLINE FString PathToVeritexServerData()
	{
		return FPaths::ProjectSavedDir() + "Veritex_Server_Data/";
	}

	static FORCEINLINE FString PathToUniverseData()
	{
		FString PathToUniverse = FPaths::ProjectSavedDir() + "Veritex_Server_Data/VeritexUniverse.dat";
		return PathToUniverse;
	}

	static FORCEINLINE FString PathToCharacters()
	{
		FString PathToChars = FPaths::ProjectSavedDir() + "Veritex_Server_Data/Characters/";
		VerifyOrCreateDirectory(PathToChars);
		return PathToChars;
	}

	static FORCEINLINE bool VerifyOrCreateDirectory(const FString& TestDir)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Directory Exists?
		if (!PlatformFile.DirectoryExists(*TestDir))
		{
			PlatformFile.CreateDirectory(*TestDir);

			if (!PlatformFile.DirectoryExists(*TestDir))
			{
				return false;
			}
		}
		return true;
	}

	UFUNCTION(BlueprintCallable, Category = Files)
		static FORCEINLINE bool FileExists(FString path)
	{
		return FPaths::FileExists(path);
	}

	UFUNCTION(BlueprintCallable, Category = Files)
		static FORCEINLINE bool FolderExists(FString Path)
	{
		return FPaths::DirectoryExists(Path);
	}

	static FORCEINLINE void SaveFile(FString OutputString, bool AllowOverwriting, FString SaveDirectory, FString FileName)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// CreateDirectoryTree returns true if the destination
		// directory existed prior to call or has been created
		// during the call.
		if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
		{
			// Get absolute file path
			FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

			// Allow overwriting or file doesn't already exist

			if (AllowOverwriting || !PlatformFile.FileExists(*AbsoluteFilePath))
			{
				FFileHelper::SaveStringToFile(OutputString, *AbsoluteFilePath);
			}
		}
	}

	// Takes Absolute File Path
	static FORCEINLINE bool DeleteFile(const FString& FileToDelete)
	{
		return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*FileToDelete);
	}

	UFUNCTION(BlueprintCallable, Category = Database)
		static FORCEINLINE FString GetSaveFileAsString(FString CompleteFilePath)
	{
		FString OutputString;

		FFileHelper::LoadFileToString(OutputString, *CompleteFilePath);
		return OutputString;
	}

	/**
	Copied From : https://answers.unrealengine.com/questions/212791/how-to-get-file-list-in-a-directory.html
	Gets all the files in a given directory.
	@param directory The full path of the directory we want to iterate over.
	@param fullpath Whether the returned list should be the full file paths or just the filenames.
	@param onlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
	@param onlyFilesEndingWith Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
	@return A list of files (including the extension).
	*/
	static FORCEINLINE TArray<FString> GetAllFilesInDirectory(const FString directory, const bool fullPath, FString onlyFilesStartingWith = "", FString onlyFilesWithExtension = "")
	{
		// Get all files in directory
		TArray<FString> directoriesToSkip;
		IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
		PlatformFile.IterateDirectory(*directory, Visitor);
		TArray<FString> files;

		for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
		{
			const FString filePath = TimestampIt.Key();
			const FString fileName = FPaths::GetCleanFilename(filePath);
			bool shouldAddFile = true;

			// Check if filename starts with required characters
			if (!onlyFilesStartingWith.IsEmpty())
			{
				const FString left = fileName.Left(onlyFilesStartingWith.Len());

				if (!(fileName.Left(onlyFilesStartingWith.Len()).Equals(onlyFilesStartingWith)))
					shouldAddFile = false;
			}

			// Check if file extension is required characters
			if (!onlyFilesWithExtension.IsEmpty())
			{
				if (!(FPaths::GetExtension(fileName, false).Equals(onlyFilesWithExtension, ESearchCase::IgnoreCase)))
					shouldAddFile = false;
			}

			// Add full path to results
			if (shouldAddFile)
			{
				files.Add(fullPath ? filePath : fileName);
			}
		}

		return files;
	}


};
