#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGS.generated.h"


UCLASS()
class SHOOTERMULTI_API ALobbyGS : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	bool ArePlayersReady();
};