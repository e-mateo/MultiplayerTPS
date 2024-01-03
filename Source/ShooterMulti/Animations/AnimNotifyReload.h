
#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyReload.generated.h"

UCLASS()
class SHOOTERMULTI_API UAnimNotifyReload : public UAnimNotify
{
	GENERATED_BODY()

public:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
