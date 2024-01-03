#include "ShooterController.h"
#include "../Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


AShooterController::AShooterController()
{
	bReplicates = true;
	SetReplicateMovement(true);
}

void AShooterController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
		ShooterCharacter->InitPlayer();
	}
}

void AShooterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AShooterController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AShooterController::MoveRight);
	InputComponent->BindAxis("LookUp", this, &AShooterController::LookUp);
	InputComponent->BindAxis("Turn", this, &AShooterController::Turn);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &AShooterController::StartSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AShooterController::EndSprint);
	InputComponent->BindAction("Aim", IE_Pressed, this, &AShooterController::StartAim);
	InputComponent->BindAction("Aim", IE_Released, this, &AShooterController::EndAim);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AShooterController::StartReload);
	InputComponent->BindAction("Punch", IE_Pressed, this, &AShooterController::Punch);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AShooterController::StartJump);
	InputComponent->BindAction("Shoot", IE_Pressed, this, &AShooterController::StartShoot);
	InputComponent->BindAction("Shoot", IE_Released, this, &AShooterController::EndShoot);
	InputComponent->BindAction("PushButton", IE_Pressed, this, &AShooterController::PushButton);
	InputComponent->BindAction("ScoreBoard", IE_Pressed, this, &AShooterController::ActivateScoreBoard);
	InputComponent->BindAction("ScoreBoard", IE_Released, this, &AShooterController::DesactivateScoreBoard);
}

void AShooterController::MoveForward(float Value)
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead() && ShooterCharacter->GetState() != EShooterCharacterState::PushButton)
	{
		if (ShooterCharacter->GetState() == EShooterCharacterState::Sprint && Value <= 0.0f)
			EndSprint();

		if (Value != 0)
		{
			FRotator Rotation = GetControlRotation();
			Rotation.Pitch = 0.f;
			Rotation.Roll = 0.f;

			//Replicated thanks to Character Movement
			ShooterCharacter->AddMovementInput(Value * Rotation.GetNormalized().Vector());
		}
	}

}
void AShooterController::MoveRight(float Value)
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead()	&& ShooterCharacter->GetState() != EShooterCharacterState::Sprint
																	&& ShooterCharacter->GetState() != EShooterCharacterState::PushButton)
	{
		if (Value != 0)
		{
			FRotator Rotation = GetControlRotation();
			Rotation.Pitch = 0.f;
			Rotation.Roll = 0.f;

			//Replicated thanks to Character Movement
			ShooterCharacter->AddMovementInput(Value * Rotation.GetNormalized().RotateVector(FVector::RightVector));
		}
	}
}
void AShooterController::LookUp(float Value)
{
	//Replicated thanks to Character Movement
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead() && ShooterCharacter->GetState() != EShooterCharacterState::PushButton)
		AddPitchInput(Value);
}

void AShooterController::Turn(float Value)
{
	//Replicated thanks to Character Movement
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead() && ShooterCharacter->GetState() != EShooterCharacterState::PushButton)
		AddYawInput(Value);
}

void AShooterController::StartSprint()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_StartSprint();
}

void AShooterController::EndSprint()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_EndSprint();
}

void AShooterController::StartJump()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_StartJump();
}

void AShooterController::EndJump()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_EndJump();
}

void AShooterController::StartAim()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_StartAim();
}

void AShooterController::EndAim()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_EndAim();
}

void AShooterController::StartShoot()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_StartShoot();
}

void AShooterController::EndShoot()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_EndShoot();
}

void AShooterController::StartReload()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_StartReload();
}

void AShooterController::EndReload()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_EndReload();
}

void AShooterController::PushButton()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_PushButton();
}

void AShooterController::Punch()
{
	if (IsValid(ShooterCharacter) && !ShooterCharacter->IsDead())
		ShooterCharacter->Server_Punch();
}

void AShooterController::ActivateScoreBoard_Implementation()
{

}

void AShooterController::DesactivateScoreBoard_Implementation()
{

}

void AShooterController::DisableInput(APlayerController* PlayerController)
{
	Super::DisableInput(PlayerController);
	
	EndSprint();
	EndAim();
	EndShoot();
}