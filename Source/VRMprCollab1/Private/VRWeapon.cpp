// Fill out your copyright notice in the Description page of Project Settings.


#include "VRWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/GameEngine.h"
#include "Math/Vector.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AVRWeapon::AVRWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void AVRWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / RateOfFire;
}

void AVRWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location

	if (!HasAuthority())
	{
		ServerFire();
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Fire in VRWeapon.cpp"));
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector CameraLocation = FVector(EyeLocation.X, EyeLocation.Y, EyeLocation.Z-60.0f);

		FVector TraceEnd = CameraLocation + (ShotDirection * 10000);

		TraceEnd.Z = CameraLocation.Z;

		//FVector sub1 = TraceEnd - CameraLocation;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Fire in VRWeapon.cpp %s"), *(sub1.ToString())));
		//UE_LOG(LogTemp, Warning, TEXT("Fire in VRWeapon.cpp %s"), *(sub1.ToString()));

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Blocking hit! Process damage
			AActor* HitActor = Hit.GetActor();

			//SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			//if (SurfaceType == SURFACE_FLESHVULNERABLE)
			//{
				ActualDamage *= 4.0f;
			//}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			//PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		//if (DebugWeaponDrawing > 0)
		//{
			DrawDebugLine(GetWorld(), FVector(CameraLocation.X-20.0f, CameraLocation.Y+15.0f, CameraLocation.Z-20.0f), TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		//}
/*
		PlayFireEffects(TracerEndPoint);

		if (HasAuthority())
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}*/

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

/*
void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}*/


void AVRWeapon::ServerFire_Implementation()
{
	Fire();
}


bool AVRWeapon::ServerFire_Validate()
{
	return true;
}

void AVRWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AVRWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}


void AVRWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("StopFire in VRWeapon.cpp"));
}

// Called every frame
void AVRWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

