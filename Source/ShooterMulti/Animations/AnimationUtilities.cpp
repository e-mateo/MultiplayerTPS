
#include "AnimationUtilities.h"
#include "Animation/AnimInstance.h"

void UAnimationUtilities::UpdateMontagePlayState(UAnimInstance* Target, UAnimMontage* Montage, bool ShouldPlay, float InPlayRate, float InBlendOutTime)
{
	if (ShouldPlay)
	{
		if (!Target->Montage_IsPlaying(Montage))
			Target->Montage_Play(Montage, InPlayRate);
	}
	else
	{
		if (Target->Montage_IsPlaying(Montage))
			Target->Montage_Stop(InBlendOutTime, Montage);
	}
}

