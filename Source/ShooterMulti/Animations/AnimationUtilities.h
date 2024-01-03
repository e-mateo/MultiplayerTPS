
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnimationUtilities.generated.h"

class UAnimInstance;
class UAnimMontage;

UCLASS()
class SHOOTERMULTI_API UAnimationUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category=Animation)
	static void UpdateMontagePlayState(UAnimInstance* Target, UAnimMontage* Montage, bool ShouldPlay, float InPlayRate = 1.0f, float OutBlendTime = 0.5f);
	
};
