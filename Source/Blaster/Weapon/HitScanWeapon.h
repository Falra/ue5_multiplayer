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
protected:
    void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const;
    void ApplyHitEffects(const FHitResult& FireHit) const;
    
    UPROPERTY(EditAnywhere)
    float Damage = 5.0f;

    UPROPERTY(EditAnywhere)
    class UParticleSystem* ImpactParticle;

    UPROPERTY(EditAnywhere)
    UParticleSystem* BeamParticle;

    UPROPERTY(EditAnywhere)
    UParticleSystem* MuzzleFlash;

    UPROPERTY(EditAnywhere)
    USoundCue* FireSound;

    UPROPERTY(EditAnywhere)
    USoundCue* HitSound;
    
};
