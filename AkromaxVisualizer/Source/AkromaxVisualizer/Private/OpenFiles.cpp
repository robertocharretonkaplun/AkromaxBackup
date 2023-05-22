// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenFiles.h"
#include "shlobj.h" 
#include "Kismet/GameplayStatics.h"
#include "Serialization/BitReader.h"
#include "Editor/UnrealEd/Public/AssetImportTask.h"
#include "AssetToolsModule.h"

#include <Runtime\Core\Public\Misc\Paths.h>
#include <Runtime\Core\Public\HAL\FileManager.h>
#include <Runtime\Core\Public\Windows\COMPointer.h>

//Limit the maximum length of a string containing file type names
#define MAX_FILETYPES_STR 4096

//Buffer large enough to store the file names.
//The value of 65536 is not an arbitrary number,
//is the maximum value that a 16-bit unsigned integer can have
#define MAX_FILENAME_STR 65536

TArray<FString> g_outFilenames;
FString g_outFolderName;
FString g_destinationPath;

bool
UOpenFiles::openFileDialog(const FString& dialogTitle, 
                           const FString& fileTypes, 
                           TArray<FString>& outFilenames) {
  WCHAR filename[MAX_FILENAME_STR];
  WCHAR pathname[MAX_FILENAME_STR];

  //Convert the forward slashes in the path name to backslashes,
  //otherwise it'll be ignored as invalid and use whatever is cached in the registry
  //FCString::Strcpy(filename, MAX_FILENAME_STR, *(defaultFile.Replace(TEXT("/"), TEXT("\\"))));
  //FCString::Strcpy(pathname, MAX_FILENAME_STR, *(FPaths::ConvertRelativePathToFull(defaultPath).Replace(TEXT("/"), TEXT("\\"))));

  //The original file type list is taken and split into an array 
  //using the "|" separator using "ParseIntoArray"
  TArray<FString> unformattedExtensions;
  fileTypes.ParseIntoArray(unformattedExtensions, TEXT("|"), true);

  //Store the length of the list of file types
  const int32 fileTypesLen = fileTypes.Len();

  //Store clean file types.
  TArray<FString> cleanExtensionList;

  //Removes unnecessary characters from the list of file types and 
  //creates an array of clean file types that can be used later
  for (int32 extensionIndex = 1; extensionIndex < unformattedExtensions.Num(); extensionIndex += 2) {
    const FString& extension = unformattedExtensions[extensionIndex];

    if (extension != TEXT("*.*")) {
      int32 wildCardIndex = extension.Find(TEXT("*"));

      cleanExtensionList.Add(wildCardIndex != INDEX_NONE ? extension.RightChop(wildCardIndex + 1) : extension);
    }
  }

  //fileTypeStr stores the file type and fileTypesPtr the memory address of the file
  WCHAR fileTypeStr[MAX_FILETYPES_STR];
  WCHAR* fileTypesPtr = nullptr;

  //We look for if you have a file and that it does not exceed the size limit
  if (fileTypesLen > 0 && fileTypesLen - 1 < MAX_FILETYPES_STR) {

    //Assigns to the address of the first element of the String
    //and copy the content of the parameter into the local variable
    fileTypesPtr = fileTypeStr;
    FCString::Strcpy(fileTypeStr, MAX_FILETYPES_STR, *fileTypes);

    //Replace all the vertical bars "|" for null "0".
    TCHAR* Pos = fileTypeStr;
    while (Pos[0] != 0) {
      if (Pos[0] == '|') {
        Pos[0] = 0;
      }

      Pos++;
    }

    //Another null terminator is added to the end of the string and
    //another null terminator is assigned after the last character in the string
    fileTypeStr[fileTypesLen] = 0;
    fileTypeStr[fileTypesLen + 1] = 0;
  }

  //Configuring and using the Windows API function 
  //GetOpenFileName() to display a dialog for selecting a file
  const void* parentWindowHandle = nullptr;

  OPENFILENAME ofn;
  FMemory::Memzero(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = (HWND)parentWindowHandle;
  ofn.lpstrFilter = fileTypesPtr;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = MAX_FILENAME_STR;
  ofn.lpstrInitialDir = pathname;
  ofn.lpstrTitle = *dialogTitle;

  //If the length of the file types is greater than zero, 
  //the default extension lpstrDefExt of the OPENFILENAME structure is set
  if (fileTypesLen > 0) {
    ofn.lpstrDefExt = &fileTypeStr[0];
  }

  //Flags in the OPENFILENAME structure are set to hide read-only files, 
  //to allow dialog resizing and file browser styling, and to specify that the directory and file must exist.
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXPLORER;
  ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  bool bSuccess = !!::GetOpenFileName(&ofn);

  if (bSuccess) {
    //The selected file name is added to the variable
    new(outFilenames) FString(filename);

    //Gets the index of the filter selected by the user in the variable
    int outFilterIndex = 0;
    outFilterIndex = ofn.nFilterIndex - 1;

    FString extension = cleanExtensionList.IsValidIndex(outFilterIndex) ? cleanExtensionList[outFilterIndex] : TEXT("");
   
    for (auto outFilenameIt = outFilenames.CreateIterator(); outFilenameIt; ++outFilenameIt) {
      FString& outFilename = *outFilenameIt;

      outFilename = outFilename.Replace(TEXT("\\"), TEXT("/"));

      g_destinationPath = FPaths::GetPath(outFilename) + TEXT("/");

      //outFilename = IFileManager::Get().ConvertToRelativePath(*outFilename);
      if (FPaths::GetExtension(outFilename).IsEmpty() && !(extension.IsEmpty())) {
        outFilename += extension;
      }

      FPaths::NormalizeFilename(outFilename);
    }
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("Error reading results of file dialog. Error: 0x%04X"));

    return bSuccess;
  }

  g_outFilenames = outFilenames;

  return bSuccess;
}

bool 
UOpenFiles::selectFolderDialog(const FString& dialogTitle, 
                               const FString& defaultPath, 
                               FString& outFolderName) {
  bool bSuccess = false;

  //An IFileOpenDialog interface is used to display 
  //the dialog and get the folder selected by the user
  TComPtr<IFileOpenDialog> fileDialog;

  //Create an instance of the IFileOpenDialog interface
  if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog,
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(&fileDialog)))) {
      {
        DWORD dwFlags = 0;
        fileDialog->GetOptions(&dwFlags);
      
        //The FOS_PICKFOLDERS enum is used so that only folders can be selected
        fileDialog->SetOptions(dwFlags | FOS_PICKFOLDERS);
      }
      
      //Define the title
      fileDialog->SetTitle(*dialogTitle);
      if (!defaultPath.IsEmpty()) {
        FString defaultWindowsPath = FPaths::ConvertRelativePathToFull(defaultPath);
        defaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);
      
        TComPtr<IShellItem> DefaultPathItem;
        if (SUCCEEDED(::SHCreateItemFromParsingName(*defaultWindowsPath,
                                                    nullptr,
                                                    IID_PPV_ARGS(&DefaultPathItem)))) {
                                                    fileDialog->SetFolder(DefaultPathItem);
        }
      }
      
      //By selecting a folder in the operating system and clicking the "OK" button, 
      //we retrieve the path of the selected folder and assign it to the "outFolderName" variable.
      const void* parentWindowHandle = nullptr;
      if (SUCCEEDED(fileDialog->Show((HWND)parentWindowHandle))) {
        TComPtr<IShellItem> Result;
      
        if (SUCCEEDED(fileDialog->GetResult(&Result))) {
          PWSTR pFilePath = nullptr;
      
          if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath))) {
            bSuccess = true;
      
            outFolderName = pFilePath;
            FPaths::NormalizeDirectoryName(outFolderName);
            ::CoTaskMemFree(pFilePath);
          }
        }
      }
  }

  g_outFolderName = outFolderName;

  return bSuccess;
}

UAssetImportTask*
UOpenFiles::createImportTask(FString& sourcePath,
                             FString& destinationPath, 
                             bool& isSuccess, 
                             FString& outInfoMsg) {
  //Create the import task obj
  UAssetImportTask* newObJTask = NewObject<UAssetImportTask>();

  //Make sure the obj was created correclty
  if (nullptr == newObJTask) {
    isSuccess = false;
    outInfoMsg = FString::Printf(TEXT("Create the import task FAILED. - %s"), *sourcePath);

    return nullptr;
  }

  //Set the path info
  newObJTask->Filename = FPaths::GetCleanFilename(sourcePath);
  newObJTask->DestinationPath = FPaths::GetPath(destinationPath);
  newObJTask->DestinationName = "NewFileSuccess";

  //Set basic options
  newObJTask->bSave = false;
  newObJTask->bAutomated = true;
  newObJTask->bAsync = false;
  newObJTask->bReplaceExisting = true;
  newObJTask->bReplaceExistingSettings = false;

  //Return the final task
  isSuccess = true;
  outInfoMsg = FString::Printf(TEXT("Create the import task SUCCESS. - %s"), *sourcePath);

  return newObJTask;
}

void
UOpenFiles::processImportTask(UAssetImportTask* importTask,
                              bool& isSuccess,
                              FString& outInfoMsg) {
  //Make sure the import task have something to avoid errors
  if (nullptr == importTask) {
    isSuccess = false;
    outInfoMsg = FString::Printf(TEXT("Process the import task FAILED. The importTask is NULL"));
    //return isSuccess;
  }

  //Get the AssetTool module
  FAssetToolsModule* newAssetTools = FModuleManager::LoadModulePtr<FAssetToolsModule>("AssetTools");

  //Check if was created
  if (nullptr == newAssetTools) {
    isSuccess = false;
    outInfoMsg = FString::Printf(TEXT("Process the import task FAILED. The newAssetTools is NULL"),
                                      *importTask->Filename);
    //return isSuccess;
  }

  //Import the asset
  newAssetTools->Get().ImportAssetTasks({ importTask });

  //Check if anything was imported during the process
  if (0 == importTask->GetObjects().Num()) {
    isSuccess = false;
    outInfoMsg = FString::Printf(TEXT("Process the import task FAILED. Nothing was imported"),
                                      *importTask->Filename);
    //return isSuccess;
  }

  //If some imported task actually created multiple assets
  UObject* importedAsset = StaticLoadObject(UObject::StaticClass(),
                                            nullptr,
                                            *FPaths::Combine(importTask->DestinationPath,
                                                             importTask->DestinationName));
  //Return the asset
  isSuccess = true;
  outInfoMsg = FString::Printf(TEXT("Process the import task SUCCESS. %s"), *importTask->Filename);

  //return isSuccess;
}

UObject*
UOpenFiles::importAsset(FString& sourcePath,
                        FString& destinationPath,
                        bool& isSuccess,
                        FString& outInfoMsg) {
  if (destinationPath.IsEmpty()) {
    destinationPath = g_destinationPath;
  }

  UAssetImportTask* newImportTask = nullptr;
  if (sourcePath.IsEmpty()) {
    for (int32 i = 0; i < g_outFilenames.Num(); ++i) {

      //Create the import task
      newImportTask = createImportTask(g_outFilenames[i],
                                       destinationPath,
                                       isSuccess,
                                       outInfoMsg);
    }
  }
  
  //Check if import task was created
  if (!(isSuccess)) {
    outInfoMsg = FString::Printf(TEXT("Process the import asset FAILED. The isSuccess is false"));

    return nullptr;
  }

  //Import the asset
  UObject* retAsset = nullptr;

  processImportTask(newImportTask, isSuccess, outInfoMsg);

  //Check if import task was created
  if (!(isSuccess)) {
    outInfoMsg = FString::Printf(TEXT("Process the import asset FAILED. The isSuccess is false"));

    return nullptr;
  }

  //Return the imported asset
  isSuccess = true;
  outInfoMsg = FString::Printf(TEXT("Import asset SUCCESS."), *destinationPath);

  return retAsset;
}