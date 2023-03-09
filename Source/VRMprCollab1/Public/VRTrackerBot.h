// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VRTrackerBot.generated.h"

class UVRHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class VRMPRCOLLAB1_API AVRTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UVRHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(UVRHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
			AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	bool bExploded;

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
		USoundCue* SelfDestructSound;

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
		USoundCue* ExplodeSound;

	UFUNCTION(BlueprintImplementableEvent, Category = "HealthComponent")
	void BotShowUI(float HealthCng, bool BDoesDamage);

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
	bool bDoesDamage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


};
