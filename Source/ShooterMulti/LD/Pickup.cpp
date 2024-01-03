#include "Pickup.h"
#include "PickupDirector.h"
#include "../Characters/ShooterCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	BasePos = GetActorLocation();
}

void APickup::Tick(float DeltaTime)
{
	//Only do the "floating" animation on client as it has no impact on gameplay
	if (GetLocalRole() < ROLE_Authority)
	{
		GlobalTime += DeltaTime;
		float Offset = 10 * sin(GlobalTime);

		SetActorLocation(BasePos + FVector::UpVector * Offset);

		FRotator NewRot = GetActorRotation();
		NewRot.Yaw += 42 * sin(DeltaTime);
		SetActorRotation(NewRot);
	}
}

void APickup::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (GetLocalRole() != ROLE_Authority)
		return;

	AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);

	if (!IsValid(Player))
		return;

	//play the shot sound
	Multicast_PlayPickUpSound();

	if (IsValid(Director))
		Director->FreePickup(SpawnKey);
	
	Destroy();
}

void APickup::Multicast_PlayPickUpSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
}

void APickup::Reset()
{
	if(GetLocalRole() == ROLE_Authority)
		Destroy();
}

void APickup::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickup, SpawnKey);
	DOREPLIFETIME(APickup, Director);
}
