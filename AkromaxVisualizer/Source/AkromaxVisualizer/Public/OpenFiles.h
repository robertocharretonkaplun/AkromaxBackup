// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//Warnings are compiler messages that indicate possible errors or 
//problems in the code, but do not prevent the program from compiling.
#pragma warning(disable : 4668)

#include "Engine.h"
#include "AudioDevice.h"
#include "Logging/LogMacros.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OpenFiles.generated.h"

class UAssetImportTask;
class UFactory;

/**
 * 
 */
UCLASS()
class AKROMAXVISUALIZER_API UOpenFiles : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:

  /*
  * @brief Allows the display of the windows file explorer
  
  * @param It is the title of the dialog window

  * @param It is a list of file types that are displayed in the dialog window. 
           File types are separated by the | character. 
           For example, "Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

  * @param It is an output parameter that is used to store 
           the names of the files selected by the user.
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Files")
	static bool 
  openFileDialog(const FString& dialogTitle, 
                 const FString& fileTypes,
                 TArray<FString>& outFilenames);

  /*
  * @brief Allows the display of the windows file explorer
  
  * @param It is the title of the dialog window
  
  * @param It is the default path that is displayed 
           when the file selection dialog window is opened

  * @param Used to store the folder path selected by the user
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Files")
  static bool 
  selectFolderDialog(const FString& dialogTitle, 
                     const FString& defaultPath, 
                     FString& outFolderName);

  /*
  * @brief 

  * @param 
  * @param
  * @param
  * @param
  * @param
  * @param
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Files")
  static UAssetImportTask*
  createImportTask(FString& sourcePath,
                   FString& destinationPath,
                   bool& isSuccess,
                   FString& outInfoMsg);
  

  /*
  * @brief

  * @param
  * @param
  * @param
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Files")
  static void
  processImportTask(UAssetImportTask* importTask,
                    bool& isSuccess,
                    FString& outInfoMsg);

  /*
  * @brief

  * @param
  * @param
  * @param
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Files")
  static UObject* 
  importAsset(FString& sourcePath,
              FString& destinationPath,
              bool& isSuccess,
              FString& outInfoMsg);

};

/* Agregar al "Nombre del proyecto".Build.cs
* PrivateDependencyModuleNames.AddRange(new string[] { 
		  //Default modules
			"Core",
			"CoreUObject",
			"Engine",

			//New Modules
			"Json",
			"JsonUtilities",

			//
			"AssetTools",
			"UnrealED",
		});
*/