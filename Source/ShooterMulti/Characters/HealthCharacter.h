#pragma once

#include "GameFramework/Character.h"
#include "../GameFramework/ShooterPS.h"
#include "../GameFramework/Resetable.h"
#include "HealthCharacter.generated.h"

UCLASS()
class SHOOTERMULTI_API AHealthCharacter : public ACharacter, public IResetable
{
	GENERATED_BODY()

protected:
	UPROPERTY(Replicated)
	float DisapearTimer;
	UPROPERTY(Replicated)
	bool bIsDisapearing;
	TArray<UMaterialInstanceDynamic*> DissolveMaterials;

	UPROPERTY(ReplicatedUsing = Team_OnRep, BlueprintReadOnly, Category = "Character")
	ETeam Team;

	UPROPERTY(EditAnywhere, Category = "Character|Health", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.f;

	UPROPERTY(Replicated)
	float Health = MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Character|Health", meta = (ClampMin = "0.0"))
	float DisapearingDelay = 10.f;

	UPROPERTY(EditAnywhere, Category = "Character|Health", meta = (ClampMin = "0.0"))
	float DisapearingDuration = 3.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Health")
	USoundBase* HitSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Health")
	USoundBase* HeadshotHitSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Health")
	USoundBase* PunchHitSound;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character")
	class USphereComponent* PunchCollision;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character", meta = (ClampMin = "0"))
	float PunchDuration = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character", meta = (ClampMin = "0"))
	float PunchDamage = 10.f;

	void InitRagdoll();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateRagdoll();

	void ActivateRagdoll();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitSound(USoundBase* Sound, FVector Location);

public:
	DECLARE_EVENT(AHealthCharacter, TeamSwitchEvent)
	TeamSwitchEvent OnTeamSwitch;
 
	AHealthCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "Character|Health")
	bool IsDead();

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	float GetHealth() const;

	ETeam GetTeam() const;

	void SetTeam(ETeam InTeam);

	UFUNCTION()
	void Team_OnRep();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetTeam(ETeam InTeam);

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	virtual float	TakeDamage	(float					DamageAmount,
								 FDamageEvent const&	DamageEvent,
								 class AController*		EventInstigator,
								 AActor*				DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	float GainHealth(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	void InflictPunch();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_InflictPunch();

	void UpdateSkinColor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Reset() override;

	virtual void StartDisapear();
	virtual void UpdateDisapear();
	virtual void FinishDisapear();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateMaterialDisapear();

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
};