
#include "DistanceCondition.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UDistanceCondition::UDistanceCondition()
{
	NodeName = "Distance";
	Distance = 500.f;

	BlackboardKey0.AddObjectFilter(this, FName("Object"), AActor::StaticClass());
	BlackboardKey0.AddVectorFilter(this, FName("Vector"));

	BlackboardKey1.AddObjectFilter(this, FName("Object"), AActor::StaticClass());
	BlackboardKey1.AddVectorFilter(this, FName("Vector"));

}

bool UDistanceCondition::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return GetKeysDistance(OwnerComp) < Distance;
}

float UDistanceCondition::GetKeysDistance(const UBehaviorTreeComponent& OwnerComp) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	return (GetKeyPosition(Blackboard, BlackboardKey0) - GetKeyPosition(Blackboard, BlackboardKey1)).Size();
}

FVector UDistanceCondition::GetKeyPosition(const UBlackboardComponent* BlackboardComp, const FBlackboardKeySelector& BlackboardKey) const
{
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (AActor* Actor = Cast<AActor>(BlackboardComp->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID())))
			return Actor->GetActorLocation();
		
		return FVector::ZeroVector;
	}

	return BlackboardComp->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
}

void UDistanceCondition::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		BlackboardKey0.ResolveSelectedKey(*BBAsset);
		BlackboardKey1.ResolveSelectedKey(*BBAsset);
	}
	else
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("Can't initialize %s due to missing blackboard data."), *GetName());

		BlackboardKey0.InvalidateResolvedKey();
		BlackboardKey1.InvalidateResolvedKey();
	}
}

FString UDistanceCondition::GetStaticDescription() const
{
	FString Key0Desc("Invalid");
	FString Key1Desc("Invalid");

	if (BlackboardKey0.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey0.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		Key0Desc = BlackboardKey0.SelectedKeyName.ToString();
	}

	if (BlackboardKey1.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey1.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		Key1Desc = BlackboardKey1.SelectedKeyName.ToString();
	}
	
	return FString::Printf(TEXT("%s Is %s Than %f Away From %s"), *Key0Desc, IsInversed() ? TEXT("More") : TEXT("Less"), Distance, *Key1Desc);
}

#if WITH_EDITOR

FName UDistanceCondition::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.MoveDirectlyToward.Icon");
}

#endif