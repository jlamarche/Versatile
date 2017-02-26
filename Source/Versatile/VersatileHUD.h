// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "VersatileHUD.generated.h"

/**
 * 
 */
UCLASS()
class VERSATILE_API AVersatileHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AVersatileHUD();
	virtual void DrawHUD() override;
	
private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;
	
	
};
