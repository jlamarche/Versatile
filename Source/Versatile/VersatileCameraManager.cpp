// Fill out your copyright notice in the Description page of Project Settings.

#include "Versatile.h"
#include "VersatileCameraManager.h"
#include "VersatileCharacter.h"
#include "Engine.h"
#include "Math.h"

AVersatileCameraManager::AVersatileCameraManager()
{
	
}

void AVersatileCameraManager::BeginPlay()
{
	Super::BeginPlay();

}

void AVersatileCameraManager::UpdateCamera(float DeltaTime)
{
	AVersatileCharacter* Character = PCOwner ? Cast<AVersatileCharacter>(PCOwner->GetPawn()) : nullptr;
	
	if (Character != nullptr)
	{

	}
	
	Super::UpdateCamera(DeltaTime);
}
