
#pragma once

#include "Animation/AnimInstance.h"
#include "../Characters/ShooterCharacter.h"
#include "ShooterCharacterAnim.generated.h"

DECLARE_DYNAMIC_DELEGATE(FAnimEnd);

UCLASS(Transient, Blueprintable)
class SHOOTERMULTI_API UShooterCharacterAnim : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	AShooterCharacter* ShooterCharacter = nullptr;

public:
	// Properties used in the Animation Blueprint
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Shooter")
	FVector2D Direction;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Shooter")
	EShooterCharacterState State;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Shooter")
	float RelativeSpeed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	float FireAnimLength;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* ReloadMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* FireMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* FireAimMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* PunchMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* HitMontage;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation|Shooter")
	UAnimMontage* PushMontage;

	FOnMontageEnded ResetStateDelegate;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION()
	void AnimNotify_PunchHit(UAnimNotify* Notify) const;
	UFUNCTION()
	void AnimNotify_PushButton(UAnimNotify* Notify) const;
	UFUNCTION(BlueprintCallable)
	void MontageEnded() const;

	void PlayHitMontage();
	void PlayPushButtonMontage();
	void PlayPunchMontage();
};
