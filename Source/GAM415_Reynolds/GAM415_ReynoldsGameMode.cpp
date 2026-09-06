// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAM415_ReynoldsGameMode.h"
#include "GAM415_ReynoldsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGAM415_ReynoldsGameMode::AGAM415_ReynoldsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
