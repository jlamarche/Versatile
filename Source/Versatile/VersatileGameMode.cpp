// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Versatile.h"
#include "VersatileGameMode.h"
#include "VersatileHUD.h"
#include "VersatileCharacter.h"
#include "VersatilePlayerController.h"
AVersatileGameMode::AVersatileGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	
	PlayerControllerClass = AVersatilePlayerController::StaticClass();
	
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		
		HUDClass = AVersatileHUD::StaticClass();
	}
}
