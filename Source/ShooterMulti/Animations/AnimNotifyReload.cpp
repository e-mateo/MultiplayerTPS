
#include "AnimNotifyReload.h"
#include "../Characters/ShooterCharacter.h"
#include "../Controllers/ShooterController.h"


void UAnimNotifyReload::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (AShooterCharacter* Character = Cast<AShooterCharacter>(MeshComp->GetOwner()))
	{
		if (AShooterController* Controller = Cast<AShooterController>(Character->GetController()))
		{
			Controller->EndReload();
		}
	}
}
