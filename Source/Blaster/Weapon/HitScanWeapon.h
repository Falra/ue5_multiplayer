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
    FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const;
    void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const;
    
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

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float DistanceToSphere = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float SphereRadius = 75.0f;

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    bool bUseScatter = false;
};
