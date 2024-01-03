#pragma once

#include "DeathMatchGM.h"
#include "GameFramework/GameStateBase.h"
#include "DeathMatchGS.generated.h"

class AHealthCharacter;

UCLASS()
class SHOOTERMULTI_API ADeathMatchGS : public AGameStateBase
{
	GENERATED_BODY()

protected:

	FTimerHandle CountdownTimerHandle;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Shooter|GameState")
	ADeathMatchGM* GameMode = nullptr;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shooter|GameState")
	int32 CurrentTime;
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|GameState")
	int32 CurrentAICount = 0;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shooter|GameState")
	int32 RedTeamScore = 0;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shooter|GameState")
	int32 BlueTeamScore = 0;

	void AdvanceTimer();

	UFUNCTION()
	void Reset();

public:

	DECLARE_EVENT_OneParam(ADeathMatchGS, FOnPlayerAddAndRemove, ADeathMatchGS*)
	FOnPlayerAddAndRemove OnPlayerNum;
	DECLARE_EVENT_OneParam(ADeathMatchGS, TeamWinEvent, ETeam)
	TeamWinEvent OnTeamWin;
	DECLARE_EVENT(ADeathMatchGS, GameRestartEvent)
	GameRestartEvent OnGameRestart;
	GameRestartEvent OnResetAfterDelay;

	void BeginPlay() override;

	void AddScore(ETeam Team);
	void AddPlayerState(APlayerState* PlayerState) override;
	void RemovePlayerState(APlayerState* PlayerState) override;

	bool CanAddAI();
	void AddAI();
	void RemoveAI();

	int GetNbplayer();
	void UpdateEndHud(ETeam Team);

	UFUNCTION(BlueprintCallable)
	void EndGameTrigg();


	UFUNCTION(BlueprintCallable)
	void ResetAfterDelay();

	UFUNCTION(BlueprintNativeEvent)
	void ShowTeamWinHUD(ETeam Team);
	void ShowTeamWinHUD_Implementation(ETeam Team) {};
	
	int NewFrequency(int Sec);

	void NewKill(const FText& KillerName, ETeam KillerTeam, const FText& DeadName, ETeam DeadTeam);
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
};
