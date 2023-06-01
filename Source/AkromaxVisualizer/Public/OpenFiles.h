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

//Flag Enum for save multiple or single files
UENUM(BlueprintType)
enum 
dialogFlags {
  e_single = 0x00, //No flags
  e_multiple = 0x01 //Allow multiple file selections
};

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
                 const int32 flags,
                 TArray<FString>& outFilenames, 
                 int32& outFilterIndex);
};