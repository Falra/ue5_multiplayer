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

    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<class APickup>> PickupClasses;

    UPROPERTY()
    APickup* SpawnedPickup;
    
    void SpawnPickup();

    void StartSpawnPickupTimer();
    void SpawnPickupTimerFinished();

    UPROPERTY(EditAnywhere)
    float SpawnPickupTimeMin = 1.0f;
    
    UPROPERTY(EditAnywhere)
    float SpawnPickupTimeMax = 3.0f;

    UFUNCTION()
    void OnPickupDestroyed(AActor* DestroyedActor);
private:
    FTimerHandle SpawnPickupTimer;
    
};
