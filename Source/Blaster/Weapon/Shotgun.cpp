// Blaster Game. All Rights Reserved


#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
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
                
                if (ImpactParticle)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
                }

                if (HitSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
                }
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
