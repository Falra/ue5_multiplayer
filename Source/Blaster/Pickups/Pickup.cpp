// Blaster Game. All Rights Reserved


#include "Pickup.h"

APickup::APickup()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APickup::BeginPlay()
{
    Super::BeginPlay();
}

void APickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
