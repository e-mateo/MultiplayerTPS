#include "DeathMatchGS.h"
#include "ShooterPS.h"
#include "TimerManager.h"
#include "DeathMatchGM.h"
#include "../Characters/ShooterCharacter.h"
#include "../LD/Pickup.h"
#include "../Controllers/ShooterController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"


void ADeathMatchGS::BeginPlay()
{
	Super::BeginPlay();

	OnTeamWin.AddLambda([this](ETeam Team) { ShowTeamWinHUD(Team); });

	OnGameRestart.AddLambda([this]() { Reset(); });

	if (GetLocalRole() != ROLE_Authority)
		return;

	GameMode = Cast<ADeathMatchGM>(AuthorityGameMode);

	check(GameMode && "GameMode nullptr: Cast as ADeathMatchGM failed.");

	CurrentTime = GameMode->GameTime;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);

}

void ADeathMatchGS::AdvanceTimer()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	--CurrentTime;
	
	if (CurrentTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		if (RedTeamScore < BlueTeamScore)
			UpdateEndHud(ETeam::Blue);
		else if (RedTeamScore > BlueTeamScore)
			UpdateEndHud(ETeam::Red);
		else
			UpdateEndHud(ETeam::None);
	}
}

void ADeathMatchGS::AddScore(ETeam Team)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Team == ETeam::Red && ++RedTeamScore == GameMode->MaxKill)
			UpdateEndHud(ETeam::Red);
		else if (Team == ETeam::Blue && ++BlueTeamScore == GameMode->MaxKill)
			UpdateEndHud(ETeam::Blue);
	}
}

void ADeathMatchGS::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerNum.Broadcast(this);
}

void ADeathMatchGS::RemovePlayerState(APlayerState* PlayerState)
{
	OnPlayerNum.Broadcast(this);

	Super::RemovePlayerState(PlayerState);
}

bool ADeathMatchGS::CanAddAI()
{
	if (GetLocalRole() != ROLE_Authority)
		return false;

	return Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->MaxAIPerPlayer* PlayerArray.Num() > CurrentAICount;

	return false;
}

void ADeathMatchGS::AddAI()
{
	if(GetLocalRole() == ROLE_Authority)
		CurrentAICount++;
}

void ADeathMatchGS::RemoveAI()
{
	if (GetLocalRole() == ROLE_Authority)
		CurrentAICount--;
}

int ADeathMatchGS::GetNbplayer()
{
	return PlayerArray.Num();
}

void ADeathMatchGS::UpdateEndHud(ETeam Team)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		OnTeamWin.Broadcast(Team);
	}
}

void ADeathMatchGS::Reset()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> Resetables;
		UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UResetable::StaticClass(), Resetables);

		for (auto& res : Resetables)
			Cast<IResetable>(res)->Reset();

		for (TObjectPtr<APlayerState> PlayerState : PlayerArray)
		{
			APlayerState* PS = PlayerState.Get();
			if (AShooterPS* ShooterPS = Cast<AShooterPS>(PS))
			{
				ShooterPS->Client_ResetHUDKillFeed();
			}
		}
	}
}

void ADeathMatchGS::ResetAfterDelay()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentTime = GameMode->GameTime;
		GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);

		RedTeamScore = 0;
		BlueTeamScore = 0;
		CurrentAICount = 0;

		OnResetAfterDelay.Broadcast();
	}
}
void ADeathMatchGS::EndGameTrigg()
{
	OnGameRestart.Broadcast();
}

void ADeathMatchGS::NewKill(const FText& KillerName, ETeam KillerTeam, const FText& DeadName, ETeam DeadTeam)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	for (TObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		APlayerState* PS = PlayerState.Get();
		if (AShooterPS* ShooterPS = Cast<AShooterPS>(PS))
		{
			ShooterPS->Client_UpdateHUDKillFeed(KillerName, KillerTeam, DeadName, DeadTeam);
		}
	}
}
void ADeathMatchGS::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADeathMatchGS, RedTeamScore);
	DOREPLIFETIME(ADeathMatchGS, BlueTeamScore);
	DOREPLIFETIME(ADeathMatchGS, CurrentTime);
}