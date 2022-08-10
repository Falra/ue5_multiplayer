// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
    GENERATED_BODY()
public:
    virtual void Fire(const FVector& HitTarget) override;
private:
    UPROPERTY(EditAnywhere)
    uint32 NumberOfPellets = 10;
};
