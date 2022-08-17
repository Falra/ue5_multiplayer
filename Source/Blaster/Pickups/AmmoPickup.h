// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
    GENERATED_BODY()
protected:
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
    UPROPERTY(EditAnywhere)
    int32 AmmoAmount = 30;

    UPROPERTY(EditAnywhere)
    EWeaponType WeaponType;
};
