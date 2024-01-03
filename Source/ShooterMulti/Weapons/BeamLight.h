
#pragma once

#include "GameFramework/Actor.h"
class UPointLightComponent;
#include "BeamLight.generated.h"

UCLASS()
class SHOOTERMULTI_API ABeamLight : public AActor
{
	GENERATED_BODY()

private:

	float TotalLifetime = 0.f;
	float Lifetime;
	UCurveFloat* IntensityCurve;
	float Intensity;
	UPointLightComponent* LightComponent;

public:

	ABeamLight();

	UFUNCTION(BlueprintCallable, Category = Light)
	void Initialize(FVector			const	Start,
					FVector			const	End,
					FLinearColor	const	Color,
					float			const	Lifetime,
					float			const	StartIntensity = 5000.f,
					UCurveFloat*			IntensityCurve = nullptr);

	virtual void Tick(float DeltaSeconds) override;
};
