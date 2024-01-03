#pragma once

#include "Engine/World.h"

template <typename T>
USTRUCT()
struct FSpawnParameters
{
	UPROPERTY()
	TSubclassOf<T>* Blueprint = nullptr;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FActorSpawnParameters ActorParams;
};

template <typename T>
class ObjectPool
{
protected:
	UWorld* World = nullptr;
	uint32 ObjectNum = 0u;

	TArray<T*> FreeObjects;
	TArray<T*> UsedObjects;

public:
	bool bKillOnDestroy = false;

	FSpawnParameters<T> DefaultParams;

	~ObjectPool()
	{
		if (!bKillOnDestroy) // killed by World or by User.
			return;

		while (UsedObjects.Num())
			World->DestroyActor(reinterpret_cast<AActor*>(UsedObjects.Pop()));

		while (FreeObjects.Num())
			World->DestroyActor(reinterpret_cast<AActor*>(FreeObjects.Pop()));
	}

	uint32 GetObjectNum()
	{
		return ObjectNum;
	}

	void EnableActor(AActor* Actor, bool Enable)
	{
		if (!Actor)
			return;

		Actor->SetActorHiddenInGame(!Enable);
	}

	// Must be called by User.
	void BeginPlay(UWorld* InWorld, uint32 Num, const FSpawnParameters<T>& Params = FSpawnParameters<T>())
	{
		World = InWorld;
		DefaultParams = Params;

		Reserve(Num);
	}

	void Reserve(uint32 Num)
	{
		for (uint32 i = 0u; i < Num; i++)
		{
			T* Actor = !DefaultParams.Blueprint ? World->SpawnActor<T>() :
				World->SpawnActor<T>(*DefaultParams.Blueprint, DefaultParams.Position, DefaultParams.Rotation, DefaultParams.ActorParams);

			if (!Actor)
			{
			#ifdef UE_BUILD_DEBUG
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "ObjectPool::Reserve(): SpawnActor Failed!");
			#endif

				return;
			}

			ObjectNum++;

			EnableActor(Actor, false);
			FreeObjects.Add(Actor);
		}
	}

	void ReserveASync(uint32 Num, float SpawnRate = 1.0f)
	{
		FTimerHandle Handle;

		auto Lambda = [this, Handle, Num]()
		{
			Reserve(1u);

			if (--Num <= 0)
				World->GetTimerManager().ClearTimer(Handle);
		};

		World->GetTimerManager().SetTimer(Handle, Lambda, SpawnRate, true);
	}

	T* Spawn()
	{
		T* Result = nullptr;

		if (FreeObjects.Num() == 0)
		{
			if (World)
			{
				Result = !DefaultParams.Blueprint ? World->SpawnActor<T>() :
					World->SpawnActor<T>(*DefaultParams.Blueprint, DefaultParams.Position, DefaultParams.Rotation, DefaultParams.ActorParams);
			
			#ifdef UE_BUILD_DEBUG
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "ObjectPool: Instantiate");
			#endif
			}
			if (!Result)
			{
			#ifdef UE_BUILD_DEBUG
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "ObjectPool::Spawn(): Spawn Actor Failed!");
			#endif

				return nullptr;
			}
		}
		else
			Result = FreeObjects.Pop();

		EnableActor(Result, true);
		Result->SetActorLocation(DefaultParams.Position);
		Result->SetActorRotation(DefaultParams.Rotation);

		UsedObjects.Add(Result);

		return Result;
	}
	T* Spawn(float LifeTime)
	{
		T* Result = nullptr;

		if (World != nullptr)
		{
			Result = Spawn();

			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, [this, Result]() { Free(Result); }, LifeTime, false);
		}
		
		return Result;
	}

	void Free(T* Obj)
	{
		if (!Obj)
			return;

		if (UsedObjects.Remove(Obj))
		{
			EnableActor(Obj, false);
			FreeObjects.Add(Obj);
		}
		else
		{
			Obj->Destroy();

	#ifdef UE_BUILD_DEBUG
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "ObjectPool: Free bad Object");
	#endif
		}
	}
};
