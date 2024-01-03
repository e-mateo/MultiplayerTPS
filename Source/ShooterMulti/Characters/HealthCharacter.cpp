#include "HealthCharacter.h"
#include "ShooterCharacter.h"
#include "../Weapons/DamageTypePunch.h"
#include "../GameFramework/DeathMatchGS.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine.h"
#include "../Controllers/ShooterController.h"
#include "Net/UnrealNetwork.h"


AHealthCharacter::AHealthCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Create Punch Collision
	PunchCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PunchCollision"));
	PunchCollision->SetupAttachment(RootComponent);
	PunchCollision->SetRelativeLocation(FVector(80.f, 0.f, 20.f));
	PunchCollision->InitSphereRadius(25.f);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Resources/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (MeshContainer.Succeeded())
		GetMesh()->SetSkeletalMesh(MeshContainer.Object);

	Team = ETeam::None;
	
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AHealthCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		ResetHealth();
		bIsDisapearing = false;
		InitRagdoll();
	}

	OnTeamSwitch.AddLambda([this]() { UpdateSkinColor(); });
	OnTeamSwitch.Broadcast(); // First refresh.
}

void AHealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDisapearing)
		return;

	if (GetLocalRole() == ROLE_Authority)
	{
		DisapearTimer += DeltaTime;
		UpdateDisapear();
	}
}

bool AHealthCharacter::IsDead()
{
	return Health <= 0.0f;
}

float AHealthCharacter::GetMaxHealth() const
{
	return MaxHealth;
}
float AHealthCharacter::GetHealth() const
{
	return Health;
}

ETeam AHealthCharacter::GetTeam() const
{
	return Team;
}

void AHealthCharacter::UpdateSkinColor()
{
	if (Team == ETeam::None)
		GetMesh()->SetVectorParameterValueOnMaterials("TeamColor", FVector(0, 1, 0)); // Green == bad init
	else if (Team == ETeam::AI)
		GetMesh()->SetVectorParameterValueOnMaterials("TeamColor", FVector(0.24, 0.24, 0.24)); // Black
	else
		GetMesh()->SetVectorParameterValueOnMaterials("TeamColor", FVector((Team != ETeam::Blue), (Team != ETeam::Red && Team != ETeam::Blue), (Team != ETeam::Red)));
}

float AHealthCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	//Only the server applies damage
	if (GetLocalRole() != ROLE_Authority)
		return 0.f;

	if(IsDead())
		return 0.f;

	AHealthCharacter* DamagingCharacter = Cast<AHealthCharacter>(DamageCauser);

	if (!IsValid(DamagingCharacter) || IsDead() || GetTeam() == DamagingCharacter->GetTeam()) // friendly fire off.
		return 0.0f;
	
	float TotalDamage = 0.f;

	const FPointDamageEvent* PointDamageEvent = nullptr;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		PointDamageEvent = (FPointDamageEvent*)& DamageEvent;

	bool IsHeadshot = false;
	if (!IsDead())
	{
		USoundBase* CrtHitSound = PunchHitSound;

		if (PointDamageEvent && PointDamageEvent->HitInfo.PhysMaterial.Get())
		{
			TotalDamage = DamageAmount * PointDamageEvent->HitInfo.PhysMaterial->DestructibleDamageThresholdScale;

			IsHeadshot = PointDamageEvent->HitInfo.PhysMaterial->DestructibleDamageThresholdScale > 1.0f;
			CrtHitSound = IsHeadshot ? HeadshotHitSound : HitSound;
		}
		else
			TotalDamage = DamageAmount;

		//Health is replicated
		Health = FMath::Max(0.f, Health - TotalDamage); 
		//Play Hit Sounds on all clients
		Multicast_PlayHitSound(CrtHitSound, PointDamageEvent->HitInfo.Location);
	}
	
	if (IsDead())
	{
		// Update Score on GameState
		if (IsA(AShooterCharacter::StaticClass()) && (GetTeam() == ETeam::Blue || GetTeam() == ETeam::Red))
		{
			ADeathMatchGS* GameState = Cast<ADeathMatchGS>(GetWorld()->GetGameState());
			// Check if team kill another team
			if (GetTeam() != DamagingCharacter->GetTeam())
			{
				// Add score to the killed character opponent team
				GameState->AddScore(GetTeam() == ETeam::Blue ? ETeam::Red : ETeam::Blue);
				if (DamagingCharacter->IsA(AShooterCharacter::StaticClass()) && (DamagingCharacter->GetTeam() == ETeam::Red || DamagingCharacter->GetTeam() == ETeam::Blue))
				{
					AShooterPS* DamagingPlayerState = DamagingCharacter->GetPlayerState<AShooterPS>();
					DamagingPlayerState->AddKill();
					GameState->NewKill(FText::FromString(DamagingPlayerState->UserName), DamagingCharacter->GetTeam(), FText::FromString(GetPlayerState<AShooterPS>()->UserName), GetTeam());
				}

				GetPlayerState<AShooterPS>()->AddDeath();
			}
		}

		//Activate the Ragdoll on all clients
		Multicast_ActivateRagdoll();
		StartDisapear();
	}

	return TotalDamage;
}

void AHealthCharacter::Multicast_PlayHitSound_Implementation(USoundBase* Sound, FVector Location)
{
	if (Sound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

float AHealthCharacter::GainHealth(float GainAmount)
{
	if(GetLocalRole() == ROLE_Authority && !IsDead() && GainAmount > 0.0f)
		Health = FMath::Min(Health + GainAmount, MaxHealth);

	return Health;
}

void AHealthCharacter::ResetHealth()
{
	if(GetLocalRole() == ROLE_Authority)
		Health = MaxHealth;
}

void AHealthCharacter::Server_InflictPunch_Implementation()
{
	InflictPunch();
}

bool AHealthCharacter::Server_InflictPunch_Validate()
{
	return true;
}


void AHealthCharacter::InflictPunch()
{
	//Only inflict damages on the Server
	if (GetLocalRole() != ROLE_Authority)
		return;

	TArray<struct FHitResult> OutHits;
	
	FVector StartPos = GetActorLocation();
	FVector EndPos = PunchCollision->GetComponentLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	GetWorld()->SweepMultiByObjectType(OutHits, StartPos, EndPos, FQuat::Identity, 0,
									   FCollisionShape::MakeSphere(PunchCollision->GetUnscaledSphereRadius()), Params);

	TArray<AHealthCharacter*> HitActors;

	for (auto& Hit : OutHits)
	{
		AHealthCharacter* Character = Cast<AHealthCharacter>(Hit.GetActor());

		if (Character && GetTeam() != Character->GetTeam() && !HitActors.Contains(Character))
		{
			FPointDamageEvent DamageEvent = FPointDamageEvent(PunchDamage, Hit, GetActorForwardVector(), UDamageTypePunch::StaticClass());
			Character->TakeDamage(PunchDamage, DamageEvent, nullptr, this);
			HitActors.Add(Character);
		}
	}
}

void AHealthCharacter::InitRagdoll()
{
	TArray<UActorComponent*> SkeletalComponents;
	GetComponents(USkeletalMeshComponent::StaticClass(), SkeletalComponents);
	for (UActorComponent* Component : SkeletalComponents)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component);
		TArray<UMaterialInterface*> Materials = SkeletalMeshComponent->GetMaterials();

		for (int i = 0; i < Materials.Num(); ++i)
		{
			UMaterialInstanceDynamic* NewMaterial = UMaterialInstanceDynamic::Create(Materials[i], this);
			SkeletalMeshComponent->SetMaterial(i, NewMaterial);
			DissolveMaterials.Add(NewMaterial);
		}
	}

	TArray<UActorComponent*> StaticMeshComponents;
	GetComponents(UStaticMeshComponent::StaticClass(), StaticMeshComponents);

	for (UActorComponent* Component : StaticMeshComponents)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
		TArray<UMaterialInterface*> Materials = StaticMeshComponent->GetMaterials();

		for (int i = 0; i < Materials.Num(); ++i)
		{
			UMaterialInstanceDynamic* NewMaterial = UMaterialInstanceDynamic::Create(Materials[i]->GetMaterial(), this);
			StaticMeshComponent->SetMaterial(i, NewMaterial);
			DissolveMaterials.Add(NewMaterial);
		}
	}

	for (UMaterialInstanceDynamic* Material : DissolveMaterials)
	{
		Material->SetScalarParameterValue(FName(TEXT("DissolveAmmount")), 0.f);
	}
}

void AHealthCharacter::Multicast_ActivateRagdoll_Implementation()
{
	ActivateRagdoll();
}

void AHealthCharacter::ActivateRagdoll()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (GetMesh() && Capsule)
	{
		TArray<UActorComponent*> SkeletalMeshComponents;
		GetComponents(USkeletalMeshComponent::StaticClass(), SkeletalMeshComponents);

		if (SkeletalMeshComponents.Num() > 0)
		{
			Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

			for (UActorComponent* ActorComponent : SkeletalMeshComponents)
			{
				if (USkeletalMeshComponent * SkeletalMesh = Cast<USkeletalMeshComponent>(ActorComponent))
				{
					SkeletalMesh->bPauseAnims = true;

					SkeletalMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
					SkeletalMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
					SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
					SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
					SkeletalMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
					SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					SkeletalMesh->SetSimulatePhysics(true);
				}
			}
		}
	}
}

void AHealthCharacter::StartDisapear()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	//Replicated values
	DisapearTimer = 0.f;
	bIsDisapearing = true;
}

void AHealthCharacter::UpdateDisapear()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (!bIsDisapearing || DisapearTimer < DisapearingDelay)
		return;

	Multicast_UpdateMaterialDisapear();

	if (DisapearTimer > DisapearingDelay + DisapearingDuration)
	{
		bIsDisapearing = false;
		return FinishDisapear();
	}
}

void AHealthCharacter::Multicast_UpdateMaterialDisapear_Implementation()
{
	if(GetLocalRole() < ROLE_Authority)
		GetMesh()->SetScalarParameterValueOnMaterials(FName(TEXT("DissolveAmmount")), (DisapearTimer - DisapearingDelay) / DisapearingDuration);
}
void AHealthCharacter::FinishDisapear()
{
	if(GetLocalRole() == ROLE_Authority)
		Destroy();
}

void AHealthCharacter::Reset()
{
	StartDisapear();
}

void AHealthCharacter::SetTeam(ETeam InTeam)
{
	Team = InTeam;
}

void AHealthCharacter::Team_OnRep()
{
	OnTeamSwitch.Broadcast();
}

void AHealthCharacter::Server_SetTeam_Implementation(ETeam InTeam)
{
	SetTeam(InTeam);
}

bool AHealthCharacter::Server_SetTeam_Validate(ETeam InTeam)
{
	return true;
}

void AHealthCharacter::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHealthCharacter, Health);
	DOREPLIFETIME(AHealthCharacter, bIsDisapearing);
	DOREPLIFETIME(AHealthCharacter, DisapearTimer);
	DOREPLIFETIME(AHealthCharacter, Team);
}
