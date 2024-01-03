#include "PickupDirector.h"
#include "Engine/World.h"
#include "../GameFramework/DeathMatchGS.h"

APickupDirector::APickupDirector()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void APickupDirector::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		IsSpawnFullArray.SetNum(SpawnPoints.Num());
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APickupDirector::SpawnTick, SecondPerSpawn, true);
	}
}

void APickupDirector::SpawnTick()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (bIsFull)
		return;
	
	int MaxPoints = SpawnPoints.Num() - 1;
	int RandomPoint = FMath::RandRange(0, MaxPoints);
	int PrevPoint = RandomPoint;

	while (IsSpawnFullArray[RandomPoint])
	{
		RandomPoint = (RandomPoint + 1) % MaxPoints;
		if (RandomPoint == PrevPoint)
		{
			bIsFull = true;
			return;
		}
	}

	IsSpawnFullArray[RandomPoint] = true;
	SpawnPickup(CurrentPickupIndex, RandomPoint);
	CurrentPickupIndex = (CurrentPickupIndex + 1) % PickupBPs.Num();
}

void APickupDirector::SpawnPickup(int pickupIndex, int spawnPointIndex)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	auto pickupBP = PickupBPs[pickupIndex];
	auto pickupLocation = SpawnPoints[spawnPointIndex]->GetActorLocation();
	auto pickupRotation = SpawnPoints[spawnPointIndex]->GetActorRotation();

	auto Pickup = GetWorld()->SpawnActor<APickup>(pickupBP, pickupLocation, pickupRotation);

	if (Pickup)
	{
		Pickup->SpawnKey.ClassKey = pickupIndex;
		Pickup->SpawnKey.SpawnPointKey = spawnPointIndex;
		Pickup->Director = this;
	}
}

void APickupDirector::FreePickup(FSpawnKey Key)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	IsSpawnFullArray[Key.SpawnPointKey] = false;
}

void APickupDirector::SetFull(bool isFull)
{
	bIsFull = isFull;
}

void APickupDirector::Reset()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsFull = false;

		for (int i = 0; i < IsSpawnFullArray.Num(); i++)
			IsSpawnFullArray[i] = false;
	}
}
