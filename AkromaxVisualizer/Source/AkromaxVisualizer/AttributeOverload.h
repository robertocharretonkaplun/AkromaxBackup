// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AttributeOverload.generated.h"
/**
 * 
 */
UCLASS()
class AKROMAXVISUALIZER_API UAttributeOverload : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "AttributeOverload")
		static FSoundWaveData LoadAudioFile();
};
