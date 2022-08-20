// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    APickupSpawnPoint();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

};
