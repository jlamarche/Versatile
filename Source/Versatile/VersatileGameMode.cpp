// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Versatile.h"
#include "VersatileGameMode.h"
#include "VersatileCharacter.h"

AVersatileGameMode::AVersatileGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
