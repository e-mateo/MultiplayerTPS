#pragma once

#include "HealthCharacter.h"
#include "../Weapons/WeaponComponent.h"
#include "PlayerCameraComponent.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class EShooterCharacterState : uint8
{
	IdleRun,
	Aim,
	Sprint,
	Reload,
	Jump,
	Falling,
	Punch,
	Dead,
	PushButton
};

UCLASS()
class SHOOTERMULTI_API AShooterCharacter : public AHealthCharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	USpringArmComponent* SpringArm;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	UWeaponComponent* Weapon;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character|Shooter")
	EShooterCharacterState State;
	UPROPERTY(Replicated)
	EShooterCharacterState PrevState;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float AimYaw;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_UpdateAimOffsets(float Pitch, float Yaw);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPushButtonAnim();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPunchAnim();

	void Falling() override;

	void BeginPlay() override;

	void Invincibility(float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InvincibilityFX(float Duration);
	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void InvincibilityFX(float Duration);
	void InvincibilityFX_Implementation(float Duration) {};

	void InitTeamColor(ETeam InTeam);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetSpeed(float speed);

	void StartSprint();
	void EndSprint();
	void StartJump();
	void EndJump();

public:

	UPROPERTY(Replicated)
	bool bIsShooting = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float AimArmLength = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float StandardArmLength = 300.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float AimFOV = 75.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float StandardFOV = 90.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float SprintSpeed = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float AimWalkSpeed = 180.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float ReloadWalkSpeed = 200.f;

	UPROPERTY(BlueprintReadOnly)
	float RunSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MinSprintMagnitude = .3f;

	AShooterCharacter();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	EShooterCharacterState GetState() const;
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void SetState(EShooterCharacterState InState);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetState(EShooterCharacterState InState);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UWeaponComponent* GetWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UCameraComponent* GetCameraComponent();

	void InitPlayer();

	void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartSprint();
	bool Server_StartSprint_Validate() { return true; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndSprint();
	bool Server_EndSprint_Validate() { return true; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartJump();
	bool Server_StartJump_Validate() { return true; }
	UFUNCTION(Client, Reliable)
	void Client_Jump();


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndJump();
	bool Server_EndJump_Validate() { return true; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartAim();
	bool Server_StartAim_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartAim();

	UFUNCTION(Client, Reliable)
	void Client_UpdateCameraStartAim();
	UFUNCTION(Client, Reliable)
	void Client_UpdateCameraEndAim();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndAim();
	bool Server_EndAim_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndAim();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartShoot();
	bool Server_StartShoot_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartShoot();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndShoot();
	bool Server_EndShoot_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndShoot();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartReload();
	bool Server_StartReload_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndReload();
	bool Server_EndReload_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AbortReload();
	bool Server_AbortReload_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void AbortReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PushButton();
	bool Server_PushButton_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void PushButton();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_InflictPushButton();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void InflictPushButton();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Punch();
	bool Server_Punch_Validate() { return true; }
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void Punch();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void RefreshTeamHUD(ETeam InTeam);
	void RefreshTeamHUD_Implementation(ETeam InTeam) {};

	void StartDisapear() override;
	void FinishDisapear() override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
};