#include "UndeadDirector.h"
#include "../ShooterMulti.h"
#include "../GameFramework/DeathMatchGS.h"
#include "../Characters/UndeadCharacter.h"
#include "Engine/World.h"

AUndeadDirector* AUndeadDirector::Instance = nullptr;

AUndeadDirector::AUndeadDirector()
{
	bAlwaysRelevant = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AUndeadDirector::BeginPlay()
{
	Super::BeginPlay();

	// Instance only used on Server
	Instance = this;

	if (GetLocalRole() == ROLE_Authority)
	{
		if (SpawnPoints.Num() == 0)
			UE_LOG(GLogShooterMulti, Warning, TEXT("Undead Director has no spawn point."));
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AUndeadDirector::SpawnTickEnemy, SecondPerSpawn, true);
	}
}

void AUndeadDirector::Destroyed()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	if (Instance == this)
		Instance = nullptr;
}

void AUndeadDirector::SpawnEnemy(FVector pos, const FRotator& rot, ETeam Team)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	ADeathMatchGS* temp = Cast<ADeathMatchGS>(GetWorld()->GetGameState());

	if (SpawnPoints.Num() == 0 || !temp->CanAddAI())
		return;

	pos.Y += 100; // avoid in ground spawn.

	temp->AddAI();

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	auto undead = GetWorld()->SpawnActor<AUndeadCharacter>(UndeadBlueprint, pos, rot, spawnParameters);
	undead->SetTeam(Team);
}

void AUndeadDirector::SpawnTickEnemy()
{
	int rand = FMath::RandRange(0, SpawnPoints.Num() - 1);

	SpawnEnemy(SpawnPoints[rand]->GetActorLocation(), SpawnPoints[rand]->GetActorRotation());
}

AUndeadDirector* AUndeadDirector::GetInstance()
{
	return Instance;
}