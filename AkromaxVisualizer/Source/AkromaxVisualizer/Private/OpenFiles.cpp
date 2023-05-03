// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenFiles.h"
#include "shlobj.h" 
#include "Kismet/GameplayStatics.h"
#include "Serialization/BitReader.h"

#include <Runtime\Core\Public\Misc\Paths.h>
#include <Runtime\Core\Public\HAL\FileManager.h>
#include <Runtime\Core\Public\Windows\COMPointer.h>

//Limit the maximum length of a string containing file type names
#define MAX_FILETYPES_STR 4096

//Buffer large enough to store the file names.
//The value of 65536 is not an arbitrary number,
//is the maximum value that a 16-bit unsigned integer can have
#define MAX_FILENAME_STR 65536

void
UOpenFiles::openFileDialog(const FString& dialogTitle,
                           const FString& defaultPath, 
                           const FString& defaultFile,
                           const FString& fileTypes, 
                           TArray<FString>& outFilenames) {

  WCHAR filename[MAX_FILENAME_STR];
  WCHAR pathname[MAX_FILENAME_STR];

  //Convert the forward slashes in the path name to backslashes,
  //otherwise it'll be ignored as invalid and use whatever is cached in the registry
  FCString::Strcpy(filename, MAX_FILENAME_STR, *(defaultFile.Replace(TEXT("/"), TEXT("\\"))));
  FCString::Strcpy(pathname, MAX_FILENAME_STR, *(FPaths::ConvertRelativePathToFull(defaultPath).Replace(TEXT("/"), TEXT("\\"))));

  //fileTypeStr stores the file type and fileTypesPtr the memory address of the file
  WCHAR fileTypeStr[MAX_FILETYPES_STR];
  WCHAR* fileTypesPtr = nullptr;

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

  //We look for if you have a file and that it does not exceed the size limit
  if (fileTypesLen > 0 && 
      fileTypesLen - 1 < MAX_FILETYPES_STR) {

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

      outFilename = IFileManager::Get().ConvertToRelativePath(*outFilename);

      if (FPaths::GetExtension(outFilename).IsEmpty() && 
          !(extension.IsEmpty())) {
        outFilename += extension;
      }

      FPaths::NormalizeFilename(outFilename);
    }
  }
  else {
    uint32 error = ::CommDlgExtendedError();
    if (error != ERROR_SUCCESS) {
      UE_LOG(LogTemp, Warning, TEXT("Error reading results of file dialog. Error: 0x%04X"), error);
    }
  }

  if (bSuccess) {
    openFile(outFilenames);
  }
  else {
    uint32 error = ::CommDlgExtendedError();
    if (error != ERROR_SUCCESS) {
      UE_LOG(LogTemp, Warning, TEXT("Error bSuccess is false, check C++ code"), error);
    }
  }
}

void 
UOpenFiles::openFile(TArray<FString>& outFilenames) {
  TArray<uint8> rawFileData;
  rawFileData.Reserve(65536);

  if (!(outFilenames.Num())) {
    UE_LOG(LogTemp, Warning, TEXT("No files found"));
    return;
  }

  for (const FString& filename : outFilenames) {
    UE_LOG(LogTemp, Warning, TEXT("Processing file: %s"), *filename);

    if (!(FFileHelper::LoadFileToArray(rawFileData, *filename))) {
      UE_LOG(LogTemp, Warning, TEXT("Failed to load file: %s"));
      return;
    }
  }


  USoundWave* soundWave = NewObject<USoundWave>(USoundWave::StaticClass());
  if (!(soundWave)) {
    UE_LOG(LogTemp, Warning, TEXT("Failed to create USoundWave object"));
    return;
  }

  FBitReader reader(rawFileData.GetData(), rawFileData.Num());
  soundWave->Serialize(reader);
  if (reader.GetError()) {
    UE_LOG(LogTemp, Warning, TEXT("Failed to serialize USoundWave object"));
    return;
  }

  UWorld* world = nullptr;
  if (auto* audioCompo = UGameplayStatics::SpawnSound2D(world, soundWave)) {
    world = audioCompo->GetWorld();
    audioCompo->Play();
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("Failed to spawn UAudioComponent object"));
    return;
  }
}