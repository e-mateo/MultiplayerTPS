// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup.h"
#include "HealthPickup.generated.h"

UCLASS()
class SHOOTERMULTI_API AHealthPickup : public APickup
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditAnywhere, Category = "Pickup|Health")
	int Health = 50;
	
protected:

	virtual void NotifyActorBeginOverlap(AActor * OtherActor) override;
};
