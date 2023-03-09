// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Engine/GameEngine.h"
#include "VRWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "VRHealthComponent.h"
#include "GameFramework/PawnMovementComponent.h"
//#include "VRMprCollab1.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	HealthComp = CreateDefaultSubobject<UVRHealthComponent>(TEXT("HealthComp"));
	//HealthComp->OnHealthChanged.AddDynamic(this, &AVRCharacter::OnHealthChanged);
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	

	if (HasAuthority())
	{

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<AVRWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}	
}

void AVRCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("StartFire in VRCharacter.cpp"));
	}
}


void AVRCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("StopFire in VRCharacter.cpp"));
	}
}

void AVRCharacter::OnHealthChanged(UVRHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	//ShowUI(HealthDelta);

	//UE_LOG(LogTemp, Log, TEXT("OnHealthChanged in VRCharacter.cs"));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("OnHealthChanged in VRCharacter.cs")));
	

	/*
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;

		//GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Log, TEXT("OnHealthChanged inside in VRCharacter.cs"));

		//DetachFromControllerPendingDestroy();

		//SetLifeSpan(10.0f);
	}*/
}


// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Event Tick in VRCharacter.cpp"));
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVRCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AVRCharacter::StopFire);

}

void AVRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRCharacter, CurrentWeapon);
	DOREPLIFETIME(AVRCharacter, bDied);
}

