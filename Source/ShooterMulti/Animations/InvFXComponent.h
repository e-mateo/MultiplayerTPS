#pragma once

#include "Components/SceneComponent.h"
#include "InvFXComponent.generated.h"


UCLASS( ClassGroup = Custom, meta = (BlueprintSpawnableComponent) )
class SHOOTERMULTI_API UInvFXComponent : public USceneComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(ReplicatedUsing = CurrentTime_OnRep)
	float CurrentTime = 0.0f;

	UPROPERTY(Replicated)
	UStaticMeshComponent* Mesh1 = nullptr;
	UPROPERTY(Replicated)
	UStaticMeshComponent* Mesh2 = nullptr;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 3.0f;

	UInvFXComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

	UFUNCTION()
	void CurrentTime_OnRep();
};