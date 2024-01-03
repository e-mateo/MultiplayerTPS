#include "LobbyGS.h"
#include "ShooterPS.h"


bool ALobbyGS::ArePlayersReady()
{
	if (GetLocalRole() != ROLE_Authority)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("Found %d players"), PlayerArray.Num());

	for (const auto PlayerState : PlayerArray)
	{
		AShooterPS* MyPlayerState = Cast<AShooterPS>(PlayerState);
		if (MyPlayerState == nullptr || !MyPlayerState->bIsPlayerReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("NOT all players are ready"));
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("All players are ready !!"));

	return true;
}