// Blaster Game. All Rights Reserved


#include "HitScanWeapon.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
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

        if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()); BlasterCharacter && InstigatorController)
        {
            if (HasAuthority() && !bUseServerSideRewind)
            {
                UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
            }
            else if (!HasAuthority() && bUseServerSideRewind)
            {
                BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
                BlasterOwnerController = !BlasterOwnerController
                                             ? Cast<ABlasterPlayerController>(InstigatorController)
                                             : BlasterOwnerController;
                if (BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() &&
                    BlasterOwnerCharacter->IsLocallyControlled())
                {
                    BlasterOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(BlasterCharacter, Start, HitTarget,
                        BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime, this);
                }
            }
        }

        ApplyHitEffects(FireHit);

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

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const
{
    const UWorld* World = GetWorld();
    if (!World) return;
    const FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
    GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_Visibility);
    FVector BeamEnd = End;
    if (OutHit.bBlockingHit)
    {
        BeamEnd = OutHit.ImpactPoint;
    }

    DrawDebugSphere(GetWorld(), BeamEnd, 16.0f, 12, FColor::Orange, true);

    if (BeamParticle)
    {
        if (auto Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticle, TraceStart, FRotator::ZeroRotator, true))
        {
            Beam->SetVectorParameter(FName("Target"), BeamEnd);
        }
    }
}

void AHitScanWeapon::ApplyHitEffects(const FHitResult& FireHit) const
{
    if (ImpactParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
    }

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
    }
}
