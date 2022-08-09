// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
    GENERATED_BODY()
public:
    virtual void Fire(const FVector& HitTarget) override;
    void HitScanFire(const FVector& HitTarget);
private:
    UPROPERTY(EditAnywhere)
    float Damage = 5.0f;

    UPROPERTY(EditAnywhere)
    class UParticleSystem* ImpactParticle;

    UPROPERTY(EditAnywhere)
    UParticleSystem* BeamParticle;
};
