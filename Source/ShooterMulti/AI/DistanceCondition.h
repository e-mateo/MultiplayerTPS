
#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "DistanceCondition.generated.h"

UCLASS()
class SHOOTERMULTI_API UDistanceCondition : public UBTDecorator
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = DistanceCondition)
	float Distance;
	UPROPERTY(EditAnywhere, Category = DistanceCondition)
	FBlackboardKeySelector BlackboardKey0;
	UPROPERTY(EditAnywhere, Category = DistanceCondition)
	FBlackboardKeySelector BlackboardKey1;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	float GetKeysDistance(const UBehaviorTreeComponent& OwnerComp) const;
	FVector GetKeyPosition(const UBlackboardComponent* blackboardComp, const FBlackboardKeySelector& blackboardKey) const;

public:

	UDistanceCondition();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif
};
