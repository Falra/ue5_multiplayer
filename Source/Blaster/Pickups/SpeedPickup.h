// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
    GENERATED_BODY()
protected:
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

    UPROPERTY(EditAnywhere)
    float BaseSpeedBuff = 1600.f;

    UPROPERTY(EditAnywhere)
    float CrouchSpeedBuff = 850.f;

    UPROPERTY(EditAnywhere)
    float SpeedBuffTime = 15.0f;
};
