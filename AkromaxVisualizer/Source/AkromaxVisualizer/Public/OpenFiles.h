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
  
  * @param It is the default path that is displayed 
           when the file selection dialog window is opened

  * @param It is the name of the file that is displayed by 
           default when the dialog window is opened

  * @param It is a list of file types that are displayed in the dialog window. 
           File types are separated by the | character. 
           For example, "Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

  * @param It is an output parameter that is used to store 
           the names of the files selected by the user.
  */
  UFUNCTION(BlueprintCallable, Category = "Akromax Open Dialog Files")
	static void 
  openFileDialog(const FString& dialogTitle, 
                 const FString& defaultPath, 
                 const FString& defaultFile,
                 const FString& fileTypes,
                 TArray<FString>& outFilenames);

  /*
  * @brief Allows load the file opened in "openFileDialog"

  * @param It is an output parameter that is used to store
           the names of the files selected by the user.
  */
  static void
  openFile(TArray<FString>& outFilenames);
};
