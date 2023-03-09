// Fill out your copyright notice in the Description page of Project Settings.


#include "VRHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UVRHealthComponent::UVRHealthComponent()
{
	DefaultHealth = 100;

	bIsDead = false;
	Health = 100;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UVRHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UVRHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
	
}

void UVRHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void UVRHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy,
	AActor* DamageCauser)
{
	
	if (bIsDead)
	{
		return;
	}
	
	// Update health clamped
	//Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	Health = Health - Damage;

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health)));
	
	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	/*
	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}*/
}
void UVRHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRHealthComponent, Health);
}



