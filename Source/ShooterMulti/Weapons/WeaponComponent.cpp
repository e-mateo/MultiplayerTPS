#include "WeaponComponent.h"
#include "BeamLight.h"
#include "DamageTypeRifle.h"
#include "../Characters/ShooterCharacter.h"
#include "../Controllers/ShooterController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	LightPool.BeginPlay(GetWorld(), 4u);

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		AmmoCount = MaxAmmo;
		CurrentSpread = 0.f;
		if (AmmoCount > WeaponMagazineSize)
		{
			AmmoCount -= WeaponMagazineSize - LoadedAmmo;
			LoadedAmmo = WeaponMagazineSize;
		}
		else
		{
			LoadedAmmo = AmmoCount;
			AmmoCount = 0;
		}
	}

	SetIsReplicated(true);
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_UpdateMuzzleTransform(GetSocketTransform("MuzzleFlashSocket"));
	}
	else if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		ShootTimer += DeltaTime;

		//update spread
		AShooterCharacter* Character = static_cast<AShooterCharacter*>(GetOwner());
		float MinSpread = (Character->GetState() == EShooterCharacterState::Aim) ? WeaponMinSpreadAim : WeaponMinSpreadWalk;
		CurrentSpread = FMath::Max(MinSpread, CurrentSpread - WeaponSpreadRecoveryRate * DeltaTime);
	}
}

bool UWeaponComponent::Shot()
{
	//Runs on Server

	if (ShootTimer < FireRate)
		return true;

	ShootTimer = 0.f;

	if (LoadedAmmo <= 0)
		return false;

	LoadedAmmo--;

	FLaserWeaponData WeaponData;
	WeaponData.MuzzleTransform = MuzzleTransform;
	WeaponData.LookTransform = Cast<AShooterCharacter>(GetOwner())->GetCameraComponent()->GetComponentTransform();
	WeaponData.Damages = WeaponDamage;
	WeaponData.Knockback = WeaponKnockback;
	WeaponData.Spread = CurrentSpread;

	SpawnShoot(WeaponData);

	return true;
}

void UWeaponComponent::SpawnShoot(FLaserWeaponData WeaponData)
{
	//Function Used to shoot the laser and handle its collision
	FHitResult HitResult;
	bool HasHitNonCharacter = ShootLaser(GetOwner(), HitResult, WeaponData);

	//add spread
	CurrentSpread = FMath::Min(WeaponMaxSpread, CurrentSpread + WeaponSpreadPerShot);

	//apply shake
	auto PlayerController = Cast<AShooterController>(Cast<AShooterCharacter>(GetOwner())->GetController());
	if (PlayerController && ShootShake)
		PlayerController->ClientPlayCameraShake(ShootShake);

	if (HasHitNonCharacter)
	{
		//make impact decal
		Multicast_MakeImpactDecal(HitResult, ImpactDecalMat, .9f * ImpactDecalSize, 1.1f * ImpactDecalSize);

		////create impact particles
		Multicast_MakeImpactParticles(ImpactParticle, HitResult, .66f);
	}

	//Spawn FX and Sounds on clients
	Multicast_SpawnShootFX(HasHitNonCharacter, HitResult.ImpactPoint, WeaponData);
}

void UWeaponComponent::Multicast_SpawnShootFX_Implementation(bool HasHit, const FVector& ImpactPoint, FLaserWeaponData WeaponData)
{
	if (GetOwner()->GetLocalRole() != ROLE_Authority)
	{
		//make the beam visuals
		MakeLaserBeam(WeaponData.MuzzleTransform.GetLocation(), ImpactPoint, BeamParticle, BeamIntensity, FLinearColor(1.f, 0.857892f, 0.036923f), BeamIntensityCurve);

		//play the shot sound
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotSound, WeaponData.MuzzleTransform.GetLocation());

		//make muzzle smoke
		UGameplayStatics::SpawnEmitterAttached(MuzzleSmokeParticle, this, FName("MuzzleFlashSocket"));

		//play sound if gun empty
		if (LoadedAmmo == 0)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotEmptySound, GetOwner()->GetActorLocation());
	}
}

void UWeaponComponent::Reload()
{
	if (GetOwner()->GetLocalRole() != ROLE_Authority)
		return;

	if (AmmoCount > WeaponMagazineSize)
	{
		AmmoCount -= WeaponMagazineSize - LoadedAmmo;
		LoadedAmmo = WeaponMagazineSize;
	}
	else
	{
		LoadedAmmo = AmmoCount;
		AmmoCount = 0;
	}
}

void UWeaponComponent::GetAmmo(int Count)
{
	AmmoCount = FMath::Min(AmmoCount + Count, MaxAmmo);
}

// Weapon Utiliy

bool UWeaponComponent::ShootLaser(AActor* Causer, FHitResult& HitResult, const FLaserWeaponData& WeaponData)
{
	FVector LookLocation = WeaponData.LookTransform.GetLocation();
	FVector LookDirection = WeaponData.LookTransform.GetRotation().GetForwardVector();

	//apply spread
	if (WeaponData.Spread > 0.f)
		LookDirection = UKismetMathLibrary::RandomUnitVectorInConeInRadians(LookDirection, FMath::DegreesToRadians(WeaponData.Spread * .5f));

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Causer);
	CollisionParams.bTraceComplex = true;
	CollisionParams.bReturnPhysicalMaterial = true;

	//in case of actor hit
	if (GetWorld()->LineTraceSingleByChannel(HitResult,	LookLocation, LookLocation + LookDirection * WeaponData.MaxDistance,
											 ECC_Visibility , CollisionParams))
	{
		//make damages
		FPointDamageEvent DamageEvent = FPointDamageEvent(WeaponData.Damages, HitResult, LookDirection, UDamageTypeRifle::StaticClass());
		HitResult.GetActor()->TakeDamage(WeaponData.Damages, DamageEvent, nullptr, Causer);

		//push hit actors (physics)
		TArray<UActorComponent*> SkeletalMeshComponents;
		HitResult.GetActor()->GetComponents(USkeletalMeshComponent::StaticClass(), SkeletalMeshComponents);
		for (auto ActorComponent : SkeletalMeshComponents)
		{
			USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ActorComponent);
			if (SkeletalMeshComponent->IsSimulatingPhysics())
				SkeletalMeshComponent->AddForceAtLocation(LookDirection * WeaponData.Knockback, HitResult.ImpactPoint, HitResult.BoneName);
		}
		TArray<UActorComponent*> StaticMeshComponents;
		HitResult.GetActor()->GetComponents(UStaticMeshComponent::StaticClass(), StaticMeshComponents);
		for (auto ActorComponent : StaticMeshComponents)
		{
			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ActorComponent);
			if (StaticMeshComponent->IsSimulatingPhysics())
				StaticMeshComponent->AddForceAtLocation(LookDirection * 10000000, HitResult.ImpactPoint); //We had to increase the Knockback to make the cube moves
		}

		return Cast<ACharacter>(HitResult.GetActor()) == nullptr; // if collision with non character.
	}
	else //when no actor hit
	{
		HitResult.ImpactPoint = LookLocation + LookDirection * WeaponData.MaxDistance;
		HitResult.Distance = WeaponData.MaxDistance;
		return false;
	}
}

void UWeaponComponent::MakeImpactDecal(const FHitResult& FromHit, UMaterialInterface* ImpactDecalMaterial,
									   float ImpactDecalSizeMin, float ImpactDecalSizeMax)
{
	//Only spawn FX on client
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		return;

	auto StaticMeshComponent = FromHit.GetActor()->FindComponentByClass<UStaticMeshComponent>();
	if (StaticMeshComponent)
	{
		FVector DecalPos = FromHit.ImpactPoint;
		FRotator DecalRot = (FromHit.ImpactNormal.Rotation().Quaternion() * FRotator(0.f, 0.f, FMath::RandRange(-180.f, 180.f)).Quaternion()).Rotator();
		float RandomSize = FMath::RandRange(ImpactDecalSizeMin, ImpactDecalSizeMax);
		FVector DecalSize = FVector(RandomSize, RandomSize, RandomSize);

		if (StaticMeshComponent->Mobility == EComponentMobility::Static)
		{
			UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactDecalMaterial, DecalSize,
				DecalPos, DecalRot, 0.f);
			if (DecalComponent)
				DecalComponent->FadeScreenSize = 11.f;
		}
		else
		{
			UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAttached(ImpactDecalMaterial, DecalSize, StaticMeshComponent,
				NAME_None, DecalPos, DecalRot, EAttachLocation::KeepWorldPosition, 0.f);
			if (DecalComponent)
				DecalComponent->FadeScreenSize = 11.f;
		}
	}
}

void UWeaponComponent::Multicast_MakeImpactDecal_Implementation(const FHitResult& FromHit, UMaterialInterface* ImpactDecalMaterial, float ImpactDecalSizeMin, float ImpactDecalSizeMax)
{
	MakeImpactDecal(FromHit, ImpactDecalMaterial, ImpactDecalSizeMin, ImpactDecalSizeMax);
}

void UWeaponComponent::MakeLaserBeam(FVector Start, FVector End, UParticleSystem* BeamParticles,
									 float InBeamIntensity, FLinearColor Color, UCurveFloat* InBeamIntensityCurve)
{
	//Only spawn FX on client
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		return;

	FTransform ParticleTransform;
	ParticleTransform.SetLocation(Start);
	ParticleTransform.SetRotation((End - Start).Rotation().Quaternion());

	//create a beam particle
	SpawnEmitterAtLocation(BeamParticles, ParticleTransform, Start, End);

	LightPool.Spawn(0.8f)->Initialize(Start, End, Color, 0.8f, InBeamIntensity, InBeamIntensityCurve);
}

void UWeaponComponent::Multicast_MakeLaserBeam_Implementation(FVector Start, FVector End, UParticleSystem* BeamParticles, float InBeamIntensity, FLinearColor Color, UCurveFloat* InBeamIntensityCurve)
{
	MakeLaserBeam(Start, End, BeamParticle, InBeamIntensity, Color, InBeamIntensityCurve);
}

void UWeaponComponent::MakeImpactParticles(UParticleSystem* ImpactParticles, const FHitResult& FromHit, float Scale)
{
	//Only spawn FX on client
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		return;

	FTransform HitTransform;
	HitTransform.SetLocation(FromHit.ImpactPoint);
	HitTransform.SetRotation(FromHit.Normal.Rotation().Quaternion());
	HitTransform.SetScale3D(FVector(Scale, Scale, Scale));

	SpawnEmitterAtLocation(ImpactParticles, HitTransform);
}

void UWeaponComponent::Multicast_MakeImpactParticles_Implementation(UParticleSystem* ImpactParticles, const FHitResult& FromHit, float Scale)
{
	MakeImpactParticles(ImpactParticles, FromHit, Scale);
}

void UWeaponComponent::SpawnEmitterAtLocation(UParticleSystem* EmitterTemplate, const FTransform& SpawnTransform, 
											  const FVector& Source, const FVector& Target)
{
	//Only spawn FX on client
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		return;

	if (EmitterTemplate)
	{
		UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate,
																									 SpawnTransform, true,
																									 EPSCPoolMethod::AutoRelease);
		if (Source != FVector::ZeroVector && Target != FVector::ZeroVector)
		{
			ParticleSystemComponent->SetBeamSourcePoint(0, Source, 0);
			ParticleSystemComponent->SetBeamTargetPoint(0, Target, 0);
		}
	}
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, AmmoCount);
	DOREPLIFETIME(UWeaponComponent, LoadedAmmo);
	DOREPLIFETIME(UWeaponComponent, CurrentSpread);
	DOREPLIFETIME(UWeaponComponent, ShootTimer);
	DOREPLIFETIME(UWeaponComponent, MuzzleTransform);
}

void UWeaponComponent::Server_UpdateMuzzleTransform_Implementation(FTransform Transform)
{
	MuzzleTransform = Transform;
}

bool UWeaponComponent::Server_UpdateMuzzleTransform_Validate(FTransform Transform)
{
	return true;
}