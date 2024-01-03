#include "DeathMatchGM.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFrameWork/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ADeathMatchGM::Respawn(APlayerController* PlayerController)
{
	RestartPlayerAtPlayerStart(PlayerController, ChoosePlayerStart(PlayerController));
}

void ADeathMatchGM::LoadSeamless()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerTravel to Game map"));
	bUseSeamlessTravel = true;
	GetWorld()->ServerTravel("Highrise");
}

void ADeathMatchGM::Quit()
{
	FGenericPlatformMisc::RequestExit(false);
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}