// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
    
    UPROPERTY(EditAnywhere)
    class UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditAnywhere)
    class UParticleSystem* Tracer;

    UPROPERTY(VisibleAnywhere)
    class UParticleSystemComponent* TracerComponent;

    UPROPERTY(EditAnywhere)
    UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
    class USoundCue* ImpactSound;
    
public:
};
