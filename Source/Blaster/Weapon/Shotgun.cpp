// Blaster Game. All Rights Reserved


#include "Shotgun.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::ShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    AWeapon::Fire(FVector());

    auto World = GetWorld();
    if (!World) return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    AController* InstigatorController = OwnerPawn->GetController();

    if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();

        TMap<ABlasterCharacter*, uint32> HitMap;
        for (auto HitTarget : TraceHitTargets)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);
            if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
            {
                HitMap.Contains(BlasterCharacter) ? HitMap[BlasterCharacter]++ : HitMap.Emplace(BlasterCharacter, 1);
            }
            ApplyHitEffects(FireHit);
        }

        if (!InstigatorController) return;

        TArray<ABlasterCharacter*> HitCharacters;
        for (auto& [HitCharacter, Hits] : HitMap)
        {
            if (!HitCharacter) continue;

            bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
            if (HasAuthority() && bCauseAuthDamage)
            {
                UGameplayStatics::ApplyDamage(HitCharacter, Damage * Hits, InstigatorController, this, UDamageType::StaticClass());
            }
            HitCharacters.Add(HitCharacter);
        }

        if (!HasAuthority() && bUseServerSideRewind)
        {
            BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
            BlasterOwnerController = !BlasterOwnerController
                                         ? Cast<ABlasterPlayerController>(InstigatorController)
                                         : BlasterOwnerController;
            if (BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() &&
                BlasterOwnerCharacter->IsLocallyControlled())
            {
                BlasterOwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(HitCharacters, Start, TraceHitTargets,
                    BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime, this);
            }
        }
    }
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (!MuzzleFlashSocket) return;
    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

    for (uint32 i = 0; i < NumberOfPellets; ++i)
    {
        const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.0f, SphereRadius);
        const FVector EndLoc = SphereCenter + RandVector;
        const FVector ToEndLoc = EndLoc - TraceStart;
        const FVector Result = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
        HitTargets.Add(Result);
    }
}
