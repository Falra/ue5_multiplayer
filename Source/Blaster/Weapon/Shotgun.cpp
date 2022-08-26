// Blaster Game. All Rights Reserved


#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
    AWeapon::Fire(HitTarget);
    
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
        for (uint32 i = 0; i < NumberOfPellets; ++i)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);
            if (FireHit.bBlockingHit)
            {
                if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()); BlasterCharacter && HasAuthority() && InstigatorController)
                {
                    HitMap.Contains(BlasterCharacter) ? HitMap[BlasterCharacter]++ : HitMap.Emplace(BlasterCharacter, 1);
                }
                
                ApplyHitEffects(FireHit);
            }
        }

        for (auto HitPair : HitMap)
        {
            if (HitPair.Key && HasAuthority() && InstigatorController)
            {
                UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
            }
        }
    }
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets)
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
