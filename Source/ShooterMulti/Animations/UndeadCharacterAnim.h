
#pragma once

#include "Animation/AnimInstance.h"
#include "../Characters/UndeadCharacter.h"
#include "UndeadCharacterAnim.generated.h"

UCLASS()
class SHOOTERMULTI_API UUndeadCharacterAnim : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Undead")
	AUndeadCharacter* UndeadCharacter = nullptr;

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Undead")
	FVector2D Direction;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Undead")
	EUndeadCharacterState State;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Undead")
	float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Undead")
	UAnimMontage* PunchMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Undead")
	UAnimMontage* HitMontage;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION()
	void AnimNotify_PunchHit(UAnimNotify* Notify) const;
	
	void PlayPunchMontage();

	void PlayHitMontage();
};
