// Blaster Game. All Rights Reserved


#include "PickupSpawnPoint.h"

APickupSpawnPoint::APickupSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APickupSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
