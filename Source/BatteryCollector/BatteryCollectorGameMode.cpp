// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BatteryCollector.h"
#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "SpawnVolume.h"

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
	// base decay rate
	DecayRate = 0.01f;
}

void ABatteryCollectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	// find all spawn volum Actors - better way
	for (TActorIterator<ASpawnVolume> iter(GetWorld()); iter; ++iter)
	{
		SpawnVolumActors.Add(*iter);
	}

	/*
	// find all spawn volum Actors - origin way
	TArray<AActor*> FoundActors;
	// ! performance ! target: all actor from level
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
		if (SpawnVolumeActor)
		{
			SpawnVolumActors.AddUnique(SpawnVolumeActor);
		}
	}
	*/


	SetCurrentState(EBatteryPlayState::Eplaying);

	// set the score to beat
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		PowerToWin = (MyCharacter->GetInitialPower()) * 1.25f;
	}

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

 
void ABatteryCollectorGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check that we are using the battery collector character
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	// if MyCharacter is valid and 
	if (MyCharacter)
	{
		float CurrentPower = MyCharacter->GetCurrentPower();
		// If our power is greater then needed to win, set the game's state to won
		if (CurrentPower > PowerToWin)
		{
			SetCurrentState(EBatteryPlayState::EWon);
		}
		else if (CurrentPower > 0) // character's power is positive
		{
			// decrease the character's power using the decay rate
			MyCharacter->UpdatePower(-DeltaTime * DecayRate * (MyCharacter->GetInitialPower()));
		}
		else	// character's power is negetive -> game over
		{
			SetCurrentState(EBatteryPlayState::EGameOver);
		}
	}

}

float ABatteryCollectorGameMode::GetPowerToWin() const
{
	return PowerToWin;
}

// Returns the current playing state

EBatteryPlayState ABatteryCollectorGameMode::GetCurrentState() const
{
	return CurrentState;
}

// Sets a new playing state
void ABatteryCollectorGameMode::SetCurrentState(EBatteryPlayState NewState)
{
	CurrentState = NewState;
	HandleNewState(CurrentState);
}

void ABatteryCollectorGameMode::HandleNewState(EBatteryPlayState NewState)
{
	switch (NewState)
	{
	case EBatteryPlayState::Eplaying:
	{
		// spawn volume active
		for (ASpawnVolume* spawnVolume : SpawnVolumActors)
		{
			spawnVolume->SetSpawningActive(true);
		}
	}
		break;
	case EBatteryPlayState::EGameOver:
	{
		// spawn volume inactive
		for (ASpawnVolume* spawnVolume : SpawnVolumActors)
		{
			spawnVolume->SetSpawningActive(false);
		}
		// block player input
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController)
		{
			PlayerController->SetCinematicMode(true, false, false, true, true);
		}
		// ragdoll the character
		ACharacter* MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (MyCharacter)
		{
			MyCharacter->GetMesh()->SetSimulatePhysics(true);
			MyCharacter->GetMovementComponent()->MovementState.bCanJump = false;
		}
	}
		break;
	case EBatteryPlayState::EWon:
	{
		// spawn volume inactive
		for (ASpawnVolume* spawnVolume : SpawnVolumActors)
		{
			spawnVolume->SetSpawningActive(false);
		}
	}
		break;
	case EBatteryPlayState::EUnknown:
	default:
		// do nothing
		break;
	}
}
