// Blaster Game. All Rights Reserved


#include "Projectile.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
