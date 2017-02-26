// Fill out your copyright notice in the Description page of Project Settings.

#include "Versatile.h"
#include "VersatilePlayerController.h"
#include "VersatileCameraManager.h"

AVersatilePlayerController::AVersatilePlayerController()
{
	PlayerCameraManagerClass = AVersatileCameraManager::StaticClass();
}

