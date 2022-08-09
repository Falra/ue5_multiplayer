// Blaster Game. All Rights Reserved


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);
    HitScanFire(HitTarget);
}

void AHitScanWeapon::HitScanFire(const FVector& HitTarget)
{
    if (!GetWorld()) return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    AController* InstigatorController = OwnerPawn->GetController();
    if (!InstigatorController) return;
    
    if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();
        const FVector End = (HitTarget - Start) * 1.25f;
        FHitResult FireHit;
        GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
        if (!FireHit.bBlockingHit) return;
        ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
        if (!BlasterCharacter) return;
        UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
    }
}
