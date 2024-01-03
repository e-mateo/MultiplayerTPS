#pragma once

#include "GameFramework/PlayerState.h"
#include "DeathMatchGM.h"
#include "PlayerGI.h"
#include "ShooterPS.generated.h"

UCLASS()
class SHOOTERMULTI_API AShooterPS : public APlayerState
{
	GENERATED_BODY()

protected:

	void BeginPlay() override;

public:

	UPROPERTY(Replicated, BlueprintReadOnly)
	int NbKill;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int NbDeath;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int NbKillWithoutDeath;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsPlayerReady = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ETeam CurrentTeam = ETeam::None;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FString UserName = FString("UserName");

	// Used to copy properties from the current PlayerState to the passed one
	virtual void CopyProperties(class APlayerState* PlayerState);
	// Used to override the current PlayerState with the properties of the passed one
	virtual void OverrideWith(class APlayerState* PlayerState);

	UFUNCTION()
	void Reset();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_PlayerIsReady(const FPlayerInfo& PlayerInfo);

	void AddKill();
	void AddDeath();

	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerReady();

	UFUNCTION(Client, Reliable)
	void Client_OnPlayerReady();

	UFUNCTION(Client, Reliable)
	void Client_UpdateHUDKillFeed(const FText& KillerName, ETeam KillerTeam, const FText& DeadName, ETeam DeadTeam);

	UFUNCTION(Client, Reliable)
	void Client_ResetHUDKillFeed();

	UFUNCTION(Client, Reliable)
	void Client_DisplayKillStreak(int NumberKill);
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

};
