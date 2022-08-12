// Blaster Game. All Rights Reserved


#include "ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("GrenadeMesh");
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
    AActor::BeginPlay(); // Skip Projectile BeginPlay

    SpawnTrailSystem();
    StartDestroyTimer();
    ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnProjectileBounce);
}

void AProjectileGrenade::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
    }
}
