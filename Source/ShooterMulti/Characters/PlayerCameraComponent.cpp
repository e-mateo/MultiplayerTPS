/*

Old code from UE 4.27, has been removed from template when upgrading to UE 5
Improved version of SpringArm / Camera with camera shake and smoothed transition to aiming state

*/

#include "PlayerCameraComponent.h"
#include "ShooterCharacter.h"
#include "../Controllers/ShooterController.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	TargetArmLength = 300.0f;
	ProbeSize = 12.0f;
	bUsePawnControlRotation = true;

	//Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(this);
	Camera->SetRelativeLocation(FVector(30.f, 0.f, 100.f));

	TargetSpringArmLength = CameraResetDistance = TargetArmLength;
	TargetCameraFOV = CameraResetFOV = Camera->FieldOfView;

	PrimaryComponentTick.bCanEverTick = true;
}

UCameraComponent* UPlayerCameraComponent::GetCameraHandle() const
{
	return Camera;
}

void UPlayerCameraComponent::SwitchToAimCamera()
{
	CurrentTime = 0.0f;

	TargetSpringArmLength = CameraAimDistance;
	TargetCameraFOV = CameraAimFOV;
}

void UPlayerCameraComponent::SwitchToWalkCamera()
{
	CurrentTime = 0.0f;

	TargetSpringArmLength = CameraResetDistance;
	TargetCameraFOV = CameraResetFOV;
}

void UPlayerCameraComponent::ShakeCamera(uint8 State, float Intensity)
{
	AShooterController* Controller = Cast<AShooterController>(Cast<AShooterCharacter>(GetOwner())->GetController());

	if (!Controller)
		return;

	if (State == uint8(EShooterCharacterState::Sprint) && SprintShake)
		Controller->ClientStartCameraShake(SprintShake, Intensity);
	else if (RunShake)
		Controller->ClientStartCameraShake(RunShake, Intensity);
	
	if (FMath::Abs(Intensity) < .02f)
	{
		if (RunShake)
			Controller->ClientStopCameraShake(RunShake, false);
		if (SprintShake)
			Controller->ClientStopCameraShake(SprintShake, false);
	}
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!FMath::IsNearlyEqual(TargetArmLength, TargetSpringArmLength))
		TargetArmLength = FMath::Lerp(TargetArmLength, TargetSpringArmLength, LerpSpeed * DeltaTime);
	if (!FMath::IsNearlyEqual(Camera->FieldOfView, TargetCameraFOV))
		Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, TargetCameraFOV, LerpSpeed * DeltaTime);
}