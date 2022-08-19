// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AJumpPickup : public APickup
{
    GENERATED_BODY()
protected:
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

    UPROPERTY(EditAnywhere)
    float JumpSpeedBuff = 4000.f;

    UPROPERTY(EditAnywhere)
    float JumpBuffTime = 15.0f;

};
