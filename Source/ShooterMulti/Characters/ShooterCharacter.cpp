#include "ShooterCharacter.h"
#include "../Animations/ShooterCharacterAnim.h"
#include "../GameFramework/PlayerGI.h"
#include "../LD/EnemySpawnerButton.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"


AShooterCharacter::AShooterCharacter()
{
	DisapearingDelay = 1.5f;

	// Create Weapon
	Weapon = CreateDefaultSubobject<UWeaponComponent>("Rifle");

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Weapons/Rifle.Rifle'"));
	if (MeshContainer.Succeeded())
		Weapon->SetSkeletalMesh(MeshContainer.Object);

	Weapon->SetRelativeLocation(FVector(1.0f, 4.0f, -2.0f));
	Weapon->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Weapon->SetupAttachment(GetMesh(), "hand_r");
	Weapon->SetIsReplicated(true);

	// Create Sprint Arm and Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->ProbeSize = 12.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("PlayerCamera");
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeLocation(FVector(30.f, 0.f, 100.f));

	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	SetReplicateMovement(true);
	GetCharacterMovement()->SetIsReplicated(true);
	GetMesh()->SetIsReplicated(true);

}

void AShooterCharacter::BeginPlay()
{
	OnTeamSwitch.AddLambda([this]() { RefreshTeamHUD(Team); });

	Super::BeginPlay();

	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (GetLocalRole() == ROLE_Authority)
		Invincibility(Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->InvincibilityTime);
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
		return;

	if (GetLocalRole() == ROLE_Authority)
	{
		if (bIsShooting && !Weapon->Shot())
			StartReload();
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		// Anim aim offsets
		FRotator LookRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
		float NewAimPitch = UKismetMathLibrary::ClampAngle(LookRotation.Pitch, -90.f, 90.f);
		float NewAimYaw = UKismetMathLibrary::ClampAngle(LookRotation.Yaw, -90.f, 90.f);

		Server_UpdateAimOffsets(NewAimPitch, NewAimYaw);
	}
}

void AShooterCharacter::Server_UpdateAimOffsets_Implementation(float Pitch, float Yaw)
{
	AimPitch = Pitch;
	AimYaw = Yaw;
}

bool AShooterCharacter::Server_UpdateAimOffsets_Validate(float Pitch, float Yaw)
{
	return true;
}

#pragma region State
EShooterCharacterState AShooterCharacter::GetState() const
{
	return State;
}

void AShooterCharacter::Server_SetState_Implementation(EShooterCharacterState InState)
{
	SetState(InState);
}

bool AShooterCharacter::Server_SetState_Validate(EShooterCharacterState InState)
{
	return true;
}

void AShooterCharacter::SetState(EShooterCharacterState InState)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	PrevState = State;
	State = InState;
}

#pragma endregion

#pragma region Inits
void AShooterCharacter::InitPlayer()
{
	//Game Instance not available on the server
	const FPlayerInfo& PlayerInfo = GetGameInstance<UPlayerGI>()->GetUserInfo();
	if (PlayerInfo.TeamNum == 0)
	{
		//In case the PlayerInfo.TeamNum is not set at the first tick
		FTimerHandle Timer;
		GetWorldTimerManager().SetTimerForNextTick(this, &AShooterCharacter::InitPlayer);
	}
	else
	{
		InitTeamColor((ETeam)PlayerInfo.TeamNum);
	}
}

void AShooterCharacter::InitTeamColor(ETeam InTeam)
{
	Server_SetTeam(InTeam);
}

void AShooterCharacter::Invincibility(float Duration)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	Health = 100000;
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [this]() { Health = MaxHealth; }, Duration, false);
	Multicast_InvincibilityFX(Duration);
}

void AShooterCharacter::Multicast_InvincibilityFX_Implementation(float Duration)
{
	InvincibilityFX(Duration);
}


void AShooterCharacter::Multicast_SetSpeed_Implementation(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

#pragma endregion

#pragma region Actions
void AShooterCharacter::Server_StartSprint_Implementation()
{
	StartSprint();
}

void AShooterCharacter::StartSprint()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Reload)
		AbortReload();
	else if (State == EShooterCharacterState::Aim)
		EndAim();

	if (State != EShooterCharacterState::IdleRun && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		PrevState = EShooterCharacterState::Sprint;
	else
		SetState(EShooterCharacterState::Sprint);

	Multicast_SetSpeed(SprintSpeed);
}

void AShooterCharacter::Server_EndSprint_Implementation()
{
	EndSprint();
}
void AShooterCharacter::EndSprint()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State != EShooterCharacterState::Sprint && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		PrevState = EShooterCharacterState::IdleRun;
	else
		SetState(EShooterCharacterState::IdleRun);

	Multicast_SetSpeed(RunSpeed);
}

void AShooterCharacter::Server_StartJump_Implementation()
{
	StartJump();
}

void AShooterCharacter::StartJump()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAim();
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (CanJump() && (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Sprint))
	{
		SetState(EShooterCharacterState::Jump);
		//Already Replicated thanks to Character Movement
		Client_Jump();
	}
}

void AShooterCharacter::Client_Jump_Implementation()
{
	Jump();
}

void AShooterCharacter::Server_EndJump_Implementation()
{
	EndJump();
}

void AShooterCharacter::EndJump()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State != EShooterCharacterState::Jump && State != EShooterCharacterState::Falling)
		return;

	SetState(EShooterCharacterState::IdleRun);
	//Already Replicated thanks to Character Movement
	StopJumping();
}

void AShooterCharacter::Server_StartAim_Implementation()
{
	StartAim();
}

void AShooterCharacter::StartAim()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State != EShooterCharacterState::IdleRun)
		return;
	
	SetState(EShooterCharacterState::Aim);
	Multicast_SetSpeed(AimWalkSpeed);

	//Update the camera on the client since only the owning player will see it
	Client_UpdateCameraStartAim();
}

void AShooterCharacter::Server_EndAim_Implementation()
{
	EndAim();
}

void AShooterCharacter::EndAim()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State != EShooterCharacterState::Aim)
		return;

	SetState(PrevState);
	Multicast_SetSpeed(RunSpeed);

	Client_UpdateCameraEndAim();
}

void AShooterCharacter::Client_UpdateCameraStartAim_Implementation()
{
	SpringArm->TargetArmLength = AimArmLength;
	Camera->FieldOfView = AimFOV;
}

void AShooterCharacter::Client_UpdateCameraEndAim_Implementation()
{
	SpringArm->TargetArmLength = StandardArmLength;
	Camera->FieldOfView = StandardFOV;
}

void AShooterCharacter::Server_StartShoot_Implementation()
{
	StartShoot();
}

void AShooterCharacter::StartShoot()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Aim)
		bIsShooting = true;
}

void AShooterCharacter::Server_EndShoot_Implementation()
{
	EndShoot();
}

void AShooterCharacter::EndShoot()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	bIsShooting = false;
}

void AShooterCharacter::Server_StartReload_Implementation()
{
	StartReload();
}

void AShooterCharacter::StartReload()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (Weapon && Weapon->AmmoCount > 0 && Weapon->WeaponMagazineSize > Weapon->LoadedAmmo)
	{
		if (State == EShooterCharacterState::Aim)
			EndAim();
		else if (bIsShooting)
			bIsShooting = false;

		if (State != EShooterCharacterState::IdleRun)
			return;

		SetState(EShooterCharacterState::Reload);
		Multicast_SetSpeed(ReloadWalkSpeed);
	}
}

void AShooterCharacter::Server_EndReload_Implementation()
{
	EndReload();
}

void AShooterCharacter::EndReload()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	SetState(EShooterCharacterState::IdleRun);
	
	Multicast_SetSpeed(RunSpeed);

	if(Weapon)
		Weapon->Reload();
}

void AShooterCharacter::Server_AbortReload_Implementation()
{
	AbortReload();
}
void AShooterCharacter::AbortReload()
{
	if (GetLocalRole() != ROLE_Authority) //returns if not on Server
		return;

	if (State != EShooterCharacterState::Reload)
		return;

	SetState(EShooterCharacterState::IdleRun);

	Multicast_SetSpeed(RunSpeed);
}

void AShooterCharacter::Falling()
{
	Super::Falling();

	if (GetLocalRole() != ROLE_Authority)
		return;

	if (State == EShooterCharacterState::Jump)
		return;

	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAim();
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	SetState(EShooterCharacterState::Falling);
}

void AShooterCharacter::Server_PushButton_Implementation()
{
	PushButton();
}

void AShooterCharacter::PushButton()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (bIsShooting)
		bIsShooting = false;
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::PushButton);
	Multicast_PlayPushButtonAnim();
}

void AShooterCharacter::Multicast_PlayPushButtonAnim_Implementation()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPushButtonMontage();
}

void AShooterCharacter::InflictPushButton()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TSubclassOf<AEnemySpawnerButton>());

	if (OverlappingActors.Num() > 0)
	{
		AEnemySpawnerButton* Button = Cast<AEnemySpawnerButton>(OverlappingActors[0]);

		if (Button)
			Button->Activate(Team);
	}
}

void AShooterCharacter::Server_InflictPushButton_Implementation()
{
	InflictPushButton();
}

bool AShooterCharacter::Server_InflictPushButton_Validate()
{
	return true;
}

void AShooterCharacter::Server_Punch_Implementation()
{
	Punch();
}

void AShooterCharacter::Punch()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (bIsShooting)
		bIsShooting = false;
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::Punch);
	Multicast_PlayPunchAnim();
}

void AShooterCharacter::Multicast_PlayPunchAnim_Implementation()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPunchMontage();
}

#pragma endregion

void AShooterCharacter::StartDisapear()
{
	Super::StartDisapear();

	//Only runs on Server
	if (GetLocalRole() != ROLE_Authority)
		return;

	FTimerHandle Handle1;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]() { Weapon->SetVisibility(false, true); }, 3.5f, false);

	if (Controller)
	{
		APlayerController* PlayerControler = Cast<APlayerController>(Controller);
		PlayerControler->DisableInput(PlayerControler);
		
		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle2, [PlayerControler]() { PlayerControler->EnableInput(PlayerControler); }, 5.0f, false);
	}
}

void AShooterCharacter::FinishDisapear()
{
	//Only runs on Server
	if (GetLocalRole() != ROLE_Authority)
		return;

	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	Super::FinishDisapear();

	Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->Respawn(PlayerController);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCharacter, State);
	DOREPLIFETIME(AShooterCharacter, PrevState);	
	DOREPLIFETIME(AShooterCharacter, bIsShooting);
	DOREPLIFETIME(AShooterCharacter, AimPitch);
	DOREPLIFETIME(AShooterCharacter, AimYaw);
}

UWeaponComponent* AShooterCharacter::GetWeaponComponent()
{
	return Weapon;
}

UCameraComponent* AShooterCharacter::GetCameraComponent()
{
	return Camera;
}