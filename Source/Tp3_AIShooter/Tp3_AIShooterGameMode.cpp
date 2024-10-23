// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tp3_AIShooterGameMode.h"
#include "Tp3_AIShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATp3_AIShooterGameMode::ATp3_AIShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
