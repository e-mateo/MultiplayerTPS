#include "ShooterPS.h"
#include "PlayerGI.h"
#include "../Characters/ShooterCharacter.h"
#include "DeathMatchGS.h"
#include "LobbyGM.h"
#include "Net/UnrealNetwork.h"
#include "PlayerGI.h"
#include "../HUD/Highrise_HUD.h"


void AShooterPS::BeginPlay()
{
	Super::BeginPlay();

	if (ADeathMatchGS* GameState = GetWorld()->GetGameState<ADeathMatchGS>())
		GameState->OnResetAfterDelay.AddLambda([this]() { Reset(); });

	if(GetOwningController() && GetOwningController()->GetLocalRole() == ROLE_AutonomousProxy && GetGameInstance<UPlayerGI>())
	{
		if (GetGameInstance<UPlayerGI>()->GetUserInfo().TeamNum == 0)
			GetGameInstance<UPlayerGI>()->SetUserInfo((int32)CurrentTeam, UserName);
	}

	bReplicates = true;
}

void AShooterPS::CopyProperties(class APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (PlayerState)
	{
		AShooterPS* ShooterPlayerState = Cast<AShooterPS>(PlayerState);
		if (ShooterPlayerState)
		{
			ShooterPlayerState->NbKill = NbKill;
			ShooterPlayerState->NbDeath = NbDeath;
			ShooterPlayerState->UserName = UserName;
			ShooterPlayerState->NbKillWithoutDeath = NbKillWithoutDeath;
			ShooterPlayerState->CurrentTeam = CurrentTeam;
			ShooterPlayerState->bIsPlayerReady = bIsPlayerReady;
		}
	}
}

void AShooterPS::OverrideWith(class APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	if (PlayerState)
	{
		AShooterPS* ShooterPlayerState = Cast<AShooterPS>(PlayerState);

		if (ShooterPlayerState)
		{
			NbKill = ShooterPlayerState->NbKill;
			NbDeath = ShooterPlayerState->NbDeath;
			UserName = ShooterPlayerState->UserName;
			NbKillWithoutDeath = ShooterPlayerState->NbKillWithoutDeath;
			CurrentTeam = ShooterPlayerState->CurrentTeam;
			bIsPlayerReady = ShooterPlayerState->bIsPlayerReady;
		}
	}
}

void AShooterPS::AddKill()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		NbKill++;
		NbKillWithoutDeath++;

		if (NbKillWithoutDeath % 5 == 0 && NbKillWithoutDeath != 0)
			Client_DisplayKillStreak(NbKillWithoutDeath);
	}
}

void AShooterPS::AddDeath()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		NbDeath++;
		NbKillWithoutDeath = 0;
	}
}

void AShooterPS::Reset()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		NbKill = 0;
		NbDeath = 0;
	}
}

void AShooterPS::Server_PlayerIsReady_Implementation(const FPlayerInfo& PlayerInfo)
{
	bIsPlayerReady = true;
	CurrentTeam = (ETeam)PlayerInfo.TeamNum;
	UserName = PlayerInfo.UserName;

	Client_OnPlayerReady();

	ALobbyGM* MyGameMode = Cast<ALobbyGM>(GetWorld()->GetAuthGameMode());
	if (MyGameMode != nullptr)
		MyGameMode->CheckPlayersAreReady();
}

void AShooterPS::OnPlayerReady_Implementation()
{
}

void AShooterPS::Client_OnPlayerReady_Implementation()
{
	OnPlayerReady();
}

bool AShooterPS::Server_PlayerIsReady_Validate(const FPlayerInfo& PlayerInfo)
{
	return true;
}

void AShooterPS::Client_UpdateHUDKillFeed_Implementation(const FText& KillerName, ETeam KillerTeam, const FText& DeadName, ETeam DeadTeam)
{
	GetPlayerController()->GetHUD<AHighrise_HUD>()->UpdateHUDKillFeed(KillerName, KillerTeam, DeadName, DeadTeam);
}

void AShooterPS::Client_DisplayKillStreak_Implementation(int NumberKill)
{
	GetPlayerController()->GetHUD<AHighrise_HUD>()->DisplayMessageKillWithoutDeathHUD(NumberKill);
}

void AShooterPS::Client_ResetHUDKillFeed_Implementation()
{
	GetPlayerController()->GetHUD<AHighrise_HUD>()->ResetHUDKillFeed();
}
void AShooterPS::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPS, NbKill);
	DOREPLIFETIME(AShooterPS, NbDeath);
	DOREPLIFETIME(AShooterPS, NbKillWithoutDeath);
	DOREPLIFETIME(AShooterPS, CurrentTeam);
	DOREPLIFETIME(AShooterPS, UserName);
	DOREPLIFETIME(AShooterPS, bIsPlayerReady);
}