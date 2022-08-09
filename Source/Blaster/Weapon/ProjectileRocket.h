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
    virtual void BeginPlay() override;
    virtual void Destroyed() override;
    void HideAndStopRocket() const;
    void DestroyTimerFinished();

    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    class UNiagaraSystem* TrailSystem;

    UPROPERTY(VisibleAnywhere, Category = "Rocket")
    class UNiagaraComponent* TrailSystemComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    USoundCue* ProjectileLoop;

    UPROPERTY(VisibleAnywhere, Category = "Rocket")
    UAudioComponent* ProjectileLoopComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    USoundAttenuation* LoopingSoundAttenuation;

    UPROPERTY(VisibleAnywhere, Category = "Rocket")
    class URocketMovementComponent* RocketMovementComponent;
    
private:
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketMinimumDamage = 10.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketDamageInnerRadius = 200.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float RocketDamageOuterRadius = 500.0f;
    UPROPERTY(VisibleAnywhere, Category = "Rocket")
    UStaticMeshComponent* RocketMesh;
    
    FTimerHandle DestroyTimer;
    UPROPERTY(EditDefaultsOnly, Category = "Rocket")
    float DestroyTime = 3.0f;
};
