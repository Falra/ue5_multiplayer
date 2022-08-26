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
    virtual void ShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
    void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
private:
    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    uint32 NumberOfPellets = 10;
};
