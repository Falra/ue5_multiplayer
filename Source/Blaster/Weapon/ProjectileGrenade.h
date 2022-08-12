// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
    GENERATED_BODY()
public:
    AProjectileGrenade();
protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:
    UPROPERTY(EditAnywhere, Category = "Grenade")
    class USoundCue* BounceSound;
};
