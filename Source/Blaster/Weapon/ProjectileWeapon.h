// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
    GENERATED_BODY()
private:
    UPROPERTY(EditAnywhere)
    TSubclassOf<class AProjectile> ProjectileClass;
public:
    virtual void Fire(const FVector& HitTarget) override;
};
