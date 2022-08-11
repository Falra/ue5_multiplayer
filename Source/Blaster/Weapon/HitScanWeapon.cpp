// Blaster Game. All Rights Reserved


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);
    HitScanFire(HitTarget);
}

void AHitScanWeapon::HitScanFire(const FVector& HitTarget)
{
    auto World = GetWorld();
    if (!World) return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    AController* InstigatorController = OwnerPawn->GetController();
    
    if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();
        FHitResult FireHit;
        WeaponTraceHit(Start, HitTarget, FireHit);
        
        if (FireHit.bBlockingHit)
        {
            if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()); BlasterCharacter && HasAuthority() && InstigatorController)
            {
                UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
            }

            if (ImpactParticle)
            {
                UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
            }

            if (HitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
            }
        }

        if (MuzzleFlash)
        {
            UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
        }
        if (FireSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
        }
    }
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World) return;
    const FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
    GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_Visibility);
    FVector BeamEnd = End;
    if (OutHit.bBlockingHit)
    {
        BeamEnd = OutHit.ImpactPoint;
    }
    if (BeamParticle)
    {
        if (auto Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticle, TraceStart, FRotator::ZeroRotator, true))
        {
            Beam->SetVectorParameter(FName("Target"), BeamEnd);
        }
    }
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const
{
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.0f, SphereRadius);
    const FVector EndLoc = SphereCenter + RandVector;
    const FVector ToEndLoc = EndLoc - TraceStart;
    const FVector Result = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
    
    DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLoc, 4.0f, 12, FColor::Orange, true);
    DrawDebugLine(GetWorld(), TraceStart, Result, FColor::Green, true);
    
    return Result;
}

