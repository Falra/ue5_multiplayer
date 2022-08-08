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
    void SpawnDestroyEffects() const;
    virtual void Destroyed() override;

protected:
    virtual void BeginPlay() override;
    void CheckIfHitPlayer(AActor* OtherActor);

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UPROPERTY(EditAnywhere)
    class UBoxComponent* CollisionBox;
    
    UPROPERTY(EditAnywhere)
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere)
    class UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
    UParticleSystem* ImpactPlayerParticles;
    
    UPROPERTY(EditAnywhere)
    class USoundCue* ImpactSound;

    bool bHitPlayer = false;
private:
    
    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditAnywhere)
    UParticleSystem* Tracer;

    UPROPERTY(VisibleAnywhere)
    class UParticleSystemComponent* TracerComponent;
};
