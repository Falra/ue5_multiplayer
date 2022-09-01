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

#pragma region ServerSideRewind

    UPROPERTY(EditAnywhere)
    bool bUseServerSideRewind = false;

    FVector_NetQuantize TraceStart;
    FVector_NetQuantize100 InitialVelocity;

    UPROPERTY(EditAnywhere)
    float InitialSpeed = 15000.0f;

#pragma endregion

    UPROPERTY(EditAnywhere)
    float Damage = 10.0f;
    
protected:
    virtual void BeginPlay() override;
    void CheckIfHitPlayer(AActor* OtherActor);
    void StartDestroyTimer();
    void DestroyTimerFinished();
    void SpawnTrailSystem();
    void ExplodeDamage();

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UPROPERTY(EditAnywhere)
    class UBoxComponent* CollisionBox;

    UPROPERTY(EditAnywhere)
    class UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
    UParticleSystem* ImpactPlayerParticles;
    
    UPROPERTY(EditAnywhere)
    class USoundCue* ImpactSound;

    bool bHitPlayer = false;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditDefaultsOnly)
    class UNiagaraSystem* TrailSystem;

    UPROPERTY(VisibleAnywhere)
    class UNiagaraComponent* TrailSystemComponent;
    
    FTimerHandle DestroyTimer;
    UPROPERTY(EditDefaultsOnly)
    float DestroyTime = 3.0f;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ProjectileMesh;

    UPROPERTY(EditDefaultsOnly)
    float ExplodeMinimumDamage = 10.0f;

    UPROPERTY(EditDefaultsOnly)
    float ExplodeDamageInnerRadius = 200.0f;

    UPROPERTY(EditDefaultsOnly)
    float ExplodeDamageOuterRadius = 500.0f;
    
private:

    UPROPERTY(EditAnywhere)
    UParticleSystem* Tracer;

    UPROPERTY(VisibleAnywhere)
    class UParticleSystemComponent* TracerComponent;
};
