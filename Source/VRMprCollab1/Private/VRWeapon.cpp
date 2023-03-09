// Fill out your copyright notice in the Description page of Project Settings.


#include "VRWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/GameEngine.h"
#include "Math/Vector.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
//#include "HeadMountedDisplay/Public/XRSystem.h"
//#include "Engine/GameEngine/"
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"


// Sets default values
AVRWeapon::AVRWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 5.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
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
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Fire in VRWeapon.cpp"));
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		FVector ShotDirection;
		FVector CameraLocation;

		//check if it's in VR mode
		IHeadMountedDisplay* pHmd = nullptr;
		TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> pStereo = nullptr;
		if (GEngine) {
			pHmd = GEngine->XRSystem->GetHMDDevice();
			pStereo = GEngine->XRSystem->GetStereoRenderingDevice();
		}
		
		if (pHmd->IsHMDEnabled() && pHmd->IsHMDConnected() && pStereo->IsStereoEnabled()) {
			// in VR mode
			

			FVector DevicePosition;
			FQuat DeviceOrientation;
			
			GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, DeviceOrientation, DevicePosition);

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("if Fire position in VRWeapon.cpp")));
			UE_LOG(LogTemp, Warning, TEXT("if Fire position in VRWeapon.cpp"));
		
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			FVector FinalPosition = MyOwner->GetActorRotation().RotateVector(DevicePosition) + PlayerController->PlayerCameraManager->GetCameraLocation();
			
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = DeviceOrientation.Vector();
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
			CameraLocation = FinalPosition;
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("else Fire in VRWeapon.cpp")));
			UE_LOG(LogTemp, Warning, TEXT("else Fire in VRWeapon.cpp"));

			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			ShotDirection = EyeRotation.Vector();
			
			// Bullet Spread
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
			CameraLocation = FVector(EyeLocation.X, EyeLocation.Y, EyeLocation.Z - 60.0f);
		}


		FVector TraceEnd = CameraLocation + (ShotDirection * 10000);

		//TraceEnd.Z = CameraLocation.Z;

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

		//EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		
		if (GetWorld()->LineTraceSingleByChannel(Hit, FVector(CameraLocation.X - 20.0f, CameraLocation.Y + 15.0f, CameraLocation.Z - 20.0f), TraceEnd, ECC_GameTraceChannel1, QueryParams))
		{
			// Blocking hit! Process damage
			AActor* HitActor = Hit.GetActor();

			//SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			//if (SurfaceType == SURFACE_FLESHVULNERABLE)
			//{
				//ActualDamage *= 4.0f;
			//}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			//PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		//if (DebugWeaponDrawing > 0)
		//{
		DrawDebugLine(GetWorld(), FVector(CameraLocation.X-20.0f, CameraLocation.Y+15.0f, CameraLocation.Z-20.0f), TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		//}

		//PlayFireEffects(TracerEndPoint);
		
		if (HasAuthority())
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			//HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}


void AVRWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	//PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}


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
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("StopFire in VRWeapon.cpp"));
}

void AVRWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}
	/*
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientStartCameraShake(FireCamShake);
		}
	}*/
}


void AVRWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AVRWeapon, HitScanTrace, COND_SkipOwner);
}

// Called every frame
void AVRWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

