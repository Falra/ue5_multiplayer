// Blaster Game. All Rights Reserved


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (!ProjectileClass || !HasAuthority()) return;
    
    const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (!MuzzleFlashSocket) return;

    const auto SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
    const FRotator TargetRotation = ToTarget.Rotation();
    UWorld* World = GetWorld();
    if (!World) return;
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = GetOwner();
    SpawnParameters.Instigator = Cast<APawn>(GetOwner());
    World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
}
