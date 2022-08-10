// Blaster Game. All Rights Reserved


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
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
        const FVector End = Start + (HitTarget - Start) * 1.25f;
        FHitResult FireHit;
        GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);

        FVector BeamEnd = End;
        if (FireHit.bBlockingHit)
        {
            BeamEnd = FireHit.ImpactPoint;

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

        if (BeamParticle)
        {
            if (auto Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticle, SocketTransform))
            {
                Beam->SetVectorParameter(FName("Target"), BeamEnd);
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
