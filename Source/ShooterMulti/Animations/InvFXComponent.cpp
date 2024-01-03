#include "InvFXComponent.h"
#include "../ShooterMulti.h"
#include "Net/UnrealNetwork.h"


UInvFXComponent::UInvFXComponent()
{
	SetWorldLocation(FVector(0.0f, 0.0f, -100.0f));

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>("InvFX1");
	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>("InvFX2");

	Mesh1->SetupAttachment(this);
	Mesh2->SetupAttachment(this);

	Mesh1->SetWorldScale3D(FVector(2.0f));
	Mesh2->SetWorldScale3D(FVector(2.0f));
	Mesh2->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));

	ConstructorHelpers::FObjectFinder<UStaticMesh> const MeshContainer(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule'"));
	if (MeshContainer.Succeeded())
	{
		Mesh1->SetStaticMesh(MeshContainer.Object);
		Mesh2->SetStaticMesh(MeshContainer.Object);
	}
	else
		UE_LOG(GLogShooterMulti, Warning, TEXT("Failed to create InvFXComponent meshes !"));;

	ConstructorHelpers::FObjectFinder<UMaterial> const MatContainer(TEXT("Material'/Game/Resources/Effects/Materials/Weapon/M_WP_LinkGun_MF_3_new.M_WP_LinkGun_MF_3_new'"));
	if (Mesh1 && Mesh2 && MatContainer.Succeeded())
	{
		Mesh1->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MatContainer.Object);
		Mesh2->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MatContainer.Object);
	}
	else
		UE_LOG(GLogShooterMulti, Warning, TEXT("Failed to create InvFXComponent materials !"));

	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UInvFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->GetLocalRole() != ROLE_Authority)
		return;

	CurrentTime += DeltaTime;

	if (CurrentTime >= Duration)
	{
		Mesh1->DestroyComponent(true);
		Mesh2->DestroyComponent(true);
		DestroyComponent(true);
	}

}

void UInvFXComponent::CurrentTime_OnRep()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		float Alpha = (Duration - CurrentTime) / Duration;

		Mesh1->SetScalarParameterValueOnMaterials("Alpha", Alpha);
		Mesh2->SetScalarParameterValueOnMaterials("Alpha", Alpha);
	}
}

void UInvFXComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInvFXComponent, Mesh1);
	DOREPLIFETIME(UInvFXComponent, Mesh2);
	DOREPLIFETIME(UInvFXComponent, CurrentTime);
}