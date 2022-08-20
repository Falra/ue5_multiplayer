// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
    GENERATED_BODY()
protected:
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
    UPROPERTY(EditAnywhere)
    float ShieldAmount = 50.0f;

    UPROPERTY(EditAnywhere)
    float ShieldBuffTime = 3.0f;
    
};
