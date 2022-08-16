#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
    GENERATED_BODY()

public:
    APickup();
    virtual void Tick(float DeltaTime) override;
protected:
    virtual void BeginPlay() override;
public:
};
