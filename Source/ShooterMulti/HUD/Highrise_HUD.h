// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Highrise_HUD.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API AHighrise_HUD : public AHUD
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void DisplayMessageKillWithoutDeathHUD(int NbKillWithoutDeath);

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void UpdateHUDKillFeed(const FText& KillerName, ETeam KillerTeam, const FText& DeadName, ETeam DeadTeam);

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void ResetHUDKillFeed();
};
