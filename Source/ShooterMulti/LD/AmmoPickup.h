
#pragma once

#include "Pickup.h"
#include "AmmoPickup.generated.h"

UCLASS()
class SHOOTERMULTI_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

protected:

	virtual void NotifyActorBeginOverlap(AActor * OtherActor) override;

public:

	UPROPERTY(EditAnywhere, Category = "Pickup|Ammo")
	int Capacity = 20;
};