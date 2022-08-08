// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
    GENERATED_BODY()
public:
    AProjectileRocket();

protected:
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketMinimumDamage = 10.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketDamageInnerRadius = 200.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketDamageOuterRadius = 500.0f;
    UPROPERTY(VisibleAnywhere, Category = "Rocket")
    UStaticMeshComponent* RocketMesh;
};
