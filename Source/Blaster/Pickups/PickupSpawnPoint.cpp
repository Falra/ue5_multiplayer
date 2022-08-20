// Blaster Game. All Rights Reserved


#include "PickupSpawnPoint.h"

#include "Pickup.h"
#include "Kismet/KismetArrayLibrary.h"

APickupSpawnPoint::APickupSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    StartSpawnPickupTimer();
}

void APickupSpawnPoint::SpawnPickup()
{
    const int32 NumPickupClasses = PickupClasses.Num();
    if (NumPickupClasses == 0) return;
    const int32 RandIndex = FMath::RandRange(0, NumPickupClasses - 1);
    SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[RandIndex], GetActorTransform());
    
}

void APickupSpawnPoint::StartSpawnPickupTimer()
{
    const float TimeToSpawn = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
    GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, TimeToSpawn);
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
    if (HasAuthority())
    {
        SpawnPickup();
    }
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
