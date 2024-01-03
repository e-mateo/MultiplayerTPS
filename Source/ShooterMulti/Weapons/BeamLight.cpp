#include "BeamLight.h"
#include "Curves/CurveFloat.h"
#include "Components/PointLightComponent.h"

ABeamLight::ABeamLight()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = LightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
}

void ABeamLight::Initialize(FVector			const	Start,
							FVector			const	End,
							FLinearColor	const	Color,
							float			const	InLifetime,
							float			const	InIntensity,
							UCurveFloat*			InIntensityCurve)
{
	FVector BeamCenter = (Start + End) * .5f;
	FVector BeamVector = End - Start;
	FRotator BeamRotation = BeamVector.Rotation();
	float BeamLength = BeamVector.Size();

	SetActorLocation(BeamCenter);
	SetActorRotation(BeamRotation);

	LightComponent->SetLightColor(Color);
	LightComponent->SetSourceLength(BeamLength);
	LightComponent->SetIntensity(InIntensity);

	TotalLifetime = InLifetime;
	Lifetime = InLifetime;
	Intensity = InIntensity;
	IntensityCurve = InIntensityCurve;
}

void ABeamLight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsHidden())
		return;

	Lifetime -= DeltaSeconds;

	if (Lifetime > 0.0f)
	{
		if (IntensityCurve)
			LightComponent->SetIntensity(Intensity * IntensityCurve->GetFloatValue(1.f - (Lifetime / TotalLifetime)));
		else
			LightComponent->SetIntensity(Intensity * Lifetime / TotalLifetime);
	}
}
