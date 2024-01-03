#include "LobbyGM.h"
#include "LobbyGS.h"
#include "ShooterPS.h"


void ALobbyGM::LoadSeamless()
{
	bUseSeamlessTravel = true;
	GetWorld()->ServerTravel("Highrise");
}

void ALobbyGM::CheckPlayersAreReady()
{
	ALobbyGS* MyGameState = GetGameState<ALobbyGS>();
	if (MyGameState != nullptr)
	{
		if (MyGameState->ArePlayersReady())
		{
			LoadSeamless();
		}
	}
}

void ALobbyGM::BeginPlay()
{
}

void ALobbyGM::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator++)
	{
		const APlayerController* PlayerController = Iterator->Get();
		ActorList.Add(PlayerController->GetPlayerState<AShooterPS>());
	}

	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
}

void ALobbyGM::PostLogin(APlayerController* Player)
{
	Super::PostLogin(Player);
	AShooterPS* PS = Cast<AShooterPS>(Player->PlayerState);
}

void ALobbyGM::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
}

void ALobbyGM::GenericPlayerInitialization(AController* Controller)
{
	Super::GenericPlayerInitialization(Controller);
}

void ALobbyGM::Logout(AController* Exiting)
{

	const int NbPlayers = GetGameState<AGameStateBase>()->PlayerArray.Num() - 1;

	if (NbPlayers <= 0)
		OnAllClientLogout();
}
