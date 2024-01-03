#include "EnemySpawnerButton.h"
#include "../AI/UndeadDirector.h"
#include "../GameFramework/DeathMatchGS.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
AEnemySpawnerButton::AEnemySpawnerButton()
{
	bReplicates = true;
}

void AEnemySpawnerButton::BeginPlay()
{
	Super::BeginPlay();

	if (material == nullptr)
	{
		TArray<UStaticMeshComponent*> Components;
		GetComponents<UStaticMeshComponent>(Components);
		material = Components[0]->CreateAndSetMaterialInstanceDynamic(2);
	}
}

void AEnemySpawnerButton::Activate(ETeam team)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (mTeam == team)
		return;
	
	SetTeam(team);

	GetWorld()->GetTimerManager().SetTimer(mSpawnTimerHandle, this, &AEnemySpawnerButton::SpawnEnemy, SecondPerSpawn, true);
	GetWorld()->GetTimerManager().SetTimer(mResetTimerHandle, this, &AEnemySpawnerButton::Reset, TimeBeforeReset, false);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ActivateSound, GetActorLocation());
}

void AEnemySpawnerButton::SpawnEnemy()
{
	auto dir = Cast<AUndeadDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AUndeadDirector::StaticClass()));
	dir->SpawnEnemy(GetActorLocation(), GetActorRotation(), mTeam);
}

void AEnemySpawnerButton::SetTeam(ETeam team)
{
	mTeam = team;
}

void AEnemySpawnerButton::Reset()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetTeam(ETeam::None);
		GetWorldTimerManager().ClearTimer(mSpawnTimerHandle);
	}
}

void AEnemySpawnerButton::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemySpawnerButton, mTeam);
}

void AEnemySpawnerButton::Team_OnRep()
{
	if (!material)
		return;

	FLinearColor color = mTeam == ETeam::Blue ? FLinearColor::Blue : mTeam == ETeam::Red ? FLinearColor::Red : FLinearColor::Green;

	material->SetVectorParameterValue("ColorActive", color);
}
