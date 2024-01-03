
#include "UndeadCharacterAnim.h"

void UUndeadCharacterAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	UndeadCharacter = Cast<AUndeadCharacter>(TryGetPawnOwner());
}

void UUndeadCharacterAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(UndeadCharacter))
		return;

	FVector2D const Movement = static_cast<FVector2D>(UndeadCharacter->GetActorRotation().UnrotateVector(UndeadCharacter->GetVelocity()));

	Direction = Movement.GetSafeNormal();
	Speed = Movement.Size();
	State = UndeadCharacter->GetState();
}

void UUndeadCharacterAnim::PlayPunchMontage()
{
	if (!Montage_IsActive(PunchMontage))
		Montage_Play(PunchMontage, .6f);
}

void UUndeadCharacterAnim::PlayHitMontage()
{
	if (!Montage_IsActive(HitMontage))
		Montage_Play(HitMontage, 1.0f);
}

void UUndeadCharacterAnim::AnimNotify_PunchHit(UAnimNotify* Notify) const
{
	if (IsValid(UndeadCharacter))
		UndeadCharacter->InflictPunch();
}
