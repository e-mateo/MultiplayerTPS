#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGM.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API ALobbyGM : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void LoadSeamless();
	
	UFUNCTION()
	void CheckPlayersAreReady();

	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;

	UFUNCTION()
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void PostLogin(APlayerController* Player) override;

	UFUNCTION()
	virtual void GenericPlayerInitialization(AController* Controller) override;

	UFUNCTION()
	virtual void PostSeamlessTravel() override;

	UFUNCTION()
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnAllClientLogout();

};
