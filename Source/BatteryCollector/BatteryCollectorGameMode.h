// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "BatteryCollectorGameMode.generated.h"

// enum to store the current state of gameplay
UENUM(BlueprintType)
enum class EBatteryPlayState : uint8
{
	Eplaying,
	EGameOver,
	EWon,
	EUnknown
};

UCLASS(minimalapi)
class ABatteryCollectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABatteryCollectorGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// Return power needed to win - needed for the HUD
	UFUNCTION(BlueprintPure, Category = "Power")
	float GetPowerToWin() const;

	// Returns the current playing state
	UFUNCTION(BlueprintPure, Category = "Power")
	EBatteryPlayState GetCurrentState() const;

	// Sets a new playing state
	void SetCurrentState(EBatteryPlayState NewState);

protected:
	// The rate at which the character loses power
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", Meta = (BlueprintProtected = "true"))
	float DecayRate;

	// The Power needed to win the game
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", Meta = (BlueprintProtected = "true"))
	float PowerToWin;

	// The Widget class to use for our HUD screen
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	// The instance of the HUD
	UPROPERTY()		// for Garbage Collection
	class UUserWidget* CurrentWidget;

private:
	// Keeps track of the current playing state
	EBatteryPlayState CurrentState;

	TArray<class ASpawnVolume*> SpawnVolumActors;

	// Handle any function calls that rely upon changing the playing state of our game
	void HandleNewState(EBatteryPlayState NewState);
};



