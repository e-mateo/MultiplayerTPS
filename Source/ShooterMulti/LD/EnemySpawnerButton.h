
#pragma once

#include "GameFramework/Actor.h"
#include "../Characters/UndeadCharacter.h"
#include "../GameFramework/Resetable.h"
#include "EnemySpawnerButton.generated.h"

UCLASS()
class SHOOTERMULTI_API AEnemySpawnerButton : public AActor, public IResetable
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(ReplicatedUsing = Team_OnRep)
	ETeam mTeam = ETeam::None;
	FTimerHandle mSpawnTimerHandle;
	FTimerHandle mResetTimerHandle;

	virtual void BeginPlay() override;
	

public:

	UPROPERTY(EditAnywhere, BlueprintInternalUseOnly)
	float SecondPerSpawn = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintInternalUseOnly)
	float TimeBeforeReset = 30.0f;

	UPROPERTY(EditAnywhere)
	USoundBase* ActivateSound;

	UMaterialInstanceDynamic* material;

	AEnemySpawnerButton();

	void Activate(ETeam team);

	UFUNCTION()
	void SetTeam(ETeam team);

	UFUNCTION()
	void Reset() override;

	ETeam GetTeam() {return mTeam;}

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

	UFUNCTION()
	void Team_OnRep();

	void SpawnEnemy();


};
