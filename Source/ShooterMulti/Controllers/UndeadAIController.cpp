#include "UndeadAIController.h"
#include "../ShooterMulti.h"
#include "../Characters/UndeadCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void AUndeadAIController::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority && BehaviorTree && BlackboardData)
	{
		UseBlackboard(BlackboardData, BlackboardComponent);

		RunBehaviorTree(BehaviorTree);

		SetIsStun(false);
	}
	else
		UE_LOG(GLogShooterMulti, Warning, TEXT("Please edit the BehaviorTree and BlackboardData values in Blueprint"));
}

bool AUndeadAIController::Punch()
{
	if (AUndeadCharacter * UndeadCharacter = Cast<AUndeadCharacter>(GetCharacter()))
		return UndeadCharacter->Punch();

	return false;
}

void AUndeadAIController::SetIsStun(bool IsStun)
{
	BlackboardComponent->SetValueAsBool(FName("IsStun"), IsStun);
}


