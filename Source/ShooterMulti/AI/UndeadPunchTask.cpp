
#include "UndeadPunchTask.h"
#include "../Controllers/UndeadAIController.h"

UUndeadPunchTask::UUndeadPunchTask()
{
	NodeName = "Undead Punch";
}

EBTNodeResult::Type UUndeadPunchTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory)
{
	if (AUndeadAIController* Owner = Cast<AUndeadAIController>(OwnerComp.GetAIOwner()))
		return Owner->Punch() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;

	return EBTNodeResult::Failed;
}

FString UUndeadPunchTask::GetStaticDescription() const
{
	return TEXT("Undead Punch");
}