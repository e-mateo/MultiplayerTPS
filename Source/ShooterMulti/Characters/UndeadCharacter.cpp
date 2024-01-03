#include "UndeadCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "../Animations/UndeadCharacterAnim.h"
#include "../GameFramework/DeathMatchGS.h"
#include "../Controllers/UndeadAIController.h"
#include "../Weapons/DamageTypePunch.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"


AUndeadCharacter::AUndeadCharacter()
{
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	bAlwaysRelevant = true;
	SetReplicates(true);
	SetReplicateMovement(true);
}

EUndeadCharacterState AUndeadCharacter::GetState() const
{
	return State;
}

void AUndeadCharacter::SetState(EUndeadCharacterState InState)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	PrevState = State;
	State = InState;

	AUndeadAIController* AIController = Cast<AUndeadAIController>(Controller);
	if (IsValid(AIController))
	{
		if (PrevState == EUndeadCharacterState::Stun)
			AIController->SetIsStun(false);
		else if (State == EUndeadCharacterState::Stun)
			AIController->SetIsStun(true);
	}
}

void AUndeadCharacter::Server_SetState_Implementation(EUndeadCharacterState InState)
{
	SetState(InState);
}

bool AUndeadCharacter::Server_SetState_Validate(EUndeadCharacterState InState)
{
	return true;
}

// Called when the game starts or when spawned
void AUndeadCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		if(Team == ETeam::None)
			SetTeam(ETeam::AI);

		SetState(EUndeadCharacterState::IdleRun);
	}
}

void AUndeadCharacter::StartStun()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (State == EUndeadCharacterState::Stun)
		return;

	SetState(EUndeadCharacterState::Stun);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &AUndeadCharacter::EndStun, StunCooldown, false);
}

void AUndeadCharacter::PlayHitMontage()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(GetMesh());

		if (SkeletalMesh != nullptr)
		{
			UUndeadCharacterAnim* AnimInstance = Cast<UUndeadCharacterAnim>(SkeletalMesh->GetAnimInstance());
			if (AnimInstance)
				AnimInstance->PlayHitMontage();
		}
	}
}

void AUndeadCharacter::EndStun()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	SetState(EUndeadCharacterState::IdleRun);
}

bool AUndeadCharacter::Punch()
{
	if (GetLocalRole() != ROLE_Authority)
		return false;

	if (State != EUndeadCharacterState::IdleRun)
		return false;

	SetState(EUndeadCharacterState::Punch);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &AUndeadCharacter::EndPunch, PunchCooldown, false);

	Multicast_PlayPunchMontage();

	return true;
}

void AUndeadCharacter::EndPunch()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (State == EUndeadCharacterState::Punch)
		SetState(EUndeadCharacterState::IdleRun);
}

void AUndeadCharacter::Multicast_PlayPunchMontage_Implementation()
{
	USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(GetMesh());

	if (SkeletalMesh != nullptr)
	{
		UUndeadCharacterAnim* AnimInstance = Cast<UUndeadCharacterAnim>(SkeletalMesh->GetAnimInstance());
		if (AnimInstance)
			AnimInstance->PlayPunchMontage();
	}
}
void AUndeadCharacter::StartDisapear()
{
	Super::StartDisapear();

	if (GetLocalRole() == ROLE_Authority)
	{
		ADeathMatchGS* GameState = Cast<ADeathMatchGS>(GetWorld()->GetGameState());
		GameState->RemoveAI();
	}
}

void AUndeadCharacter::Reset()
{
	Super::Reset();

	Multicast_ActivateRagdoll();
}

float AUndeadCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLocalRole() != ROLE_Authority)
		return 0.f;

	float Damages = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (IsDead())
		return Damages;

	TSubclassOf<UDamageType> const DamageTypeClass = DamageEvent.DamageTypeClass;

	if (Damages > 0.0f && DamageTypeClass == UDamageTypePunch::StaticClass())
		StartStun();

	return Damages;
}

void AUndeadCharacter::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUndeadCharacter, State);
	DOREPLIFETIME(AUndeadCharacter, PrevState);
}