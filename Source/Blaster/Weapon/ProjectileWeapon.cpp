// Blaster Game. All Rights Reserved


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    UWorld* World = GetWorld();
    const auto InstigatorPawn = Cast<APawn>(GetOwner());
    
    if (!MuzzleFlashSocket || !World || !InstigatorPawn) return;

    const auto SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
    const FRotator TargetRotation = ToTarget.Rotation();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = GetOwner();
    SpawnParameters.Instigator = InstigatorPawn;

    AProjectile* SpawnedProjectile = nullptr;
    if (bUseServerSideRewind )
    {
        if (InstigatorPawn->HasAuthority()) // server
        {
            if (InstigatorPawn->IsLocallyControlled()) // server - host, use replicated projectile 
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
                SpawnedProjectile->bUseServerSideRewind = false;
                SpawnedProjectile->Damage = Damage;
            }
            else // server, not locally controlled - spawn non-replicated projectile, no SSR 
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
                SpawnedProjectile->bUseServerSideRewind = false;
            }
        }
        else // client - using SSR
        {
            if (InstigatorPawn->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR 
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
                SpawnedProjectile->bUseServerSideRewind = true;
                SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
                SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
                SpawnedProjectile->Damage = Damage;
            }
            else  // client, not locally controlled - spawn non-replicated projectile, no SSR 
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
                SpawnedProjectile->bUseServerSideRewind = false;
            }
        }
    }
    else
    {
        
    }
   

    
}
