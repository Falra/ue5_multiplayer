// Blaster Game. All Rights Reserved


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
    }

    if (TrailSystem)
    {
        TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition, false);
    }

    if (ProjectileLoop && LoopingSoundAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), FName(), GetActorLocation(),
            GetActorRotation(), EAttachLocation::KeepWorldPosition, false, 1.0f, 1.0f, 0.0f, LoopingSoundAttenuation,
            (USoundConcurrency*)nullptr, false);
    }
}

void AProjectileRocket::HideAndStopRocket() const
{
    if (RocketMesh)
    {
        RocketMesh->SetVisibility(false);
    }
    if (CollisionBox)
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
    {
        TrailSystemComponent->GetSystemInstanceController()->Deactivate();
    }
    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
    }
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    CheckIfHitPlayer(OtherActor);
    if (const APawn* FiringPawn = GetInstigator(); FiringPawn && HasAuthority())
    {
        if (AController* FiringController = FiringPawn->GetController())
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, RocketMinimumDamage, GetActorLocation(), RocketDamageInnerRadius,
                RocketDamageOuterRadius, 1.0f, UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController);
        }
    }
    SpawnDestroyEffects();
    HideAndStopRocket();
    GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);
}

void AProjectileRocket::DestroyTimerFinished()
{
    Destroy();
}

void AProjectileRocket::Destroyed()
{
    // Do nothing because all effects was already spawned
}
