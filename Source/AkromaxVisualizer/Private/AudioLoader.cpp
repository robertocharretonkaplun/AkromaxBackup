// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioLoader.h"
#include "Misc/FileHelper.h"
#include "AudioDevice.h"

void UAudioLoader::LoadAudioFile(FString FilePath, TArray<uint8>& OutData, int32& OutSampleRate)
{
    // Load the audio file using the audio device
    FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw();
    if (!AudioDevice)
    {
        UE_LOG(LogTemp, Error, TEXT("No audio device found"));
        return;
    }

    // Load the audio file into memory
    TArray<uint8> AudioData;
    if (!FFileHelper::LoadFileToArray(AudioData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load audio file %s"), *FilePath);
        return;
    }

    // Parse the audio data
    OutData = AudioData;
    OutSampleRate = AudioDevice->GetSampleRate();
}