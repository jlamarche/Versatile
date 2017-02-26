// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "VersatileCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class VERSATILE_API AVersatileCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	AVersatileCameraManager();
	
	virtual void BeginPlay() override;
	
	virtual void UpdateCamera(float DeltaTime) override;
	
};
