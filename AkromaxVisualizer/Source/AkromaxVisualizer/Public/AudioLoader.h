// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AudioLoader.generated.h"

/**
 * 
 */
UCLASS()
class AKROMAXVISUALIZER_API UAudioLoader : public UObject
{
	GENERATED_BODY()

	public:
	 UFUNCTION(BlueprintCallable, Category = "Audio")
    static void LoadAudioFile(FString FilePath, TArray<uint8>& OutData, int32& OutSampleRate);

	
};
