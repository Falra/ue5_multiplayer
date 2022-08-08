// Blaster Game. All Rights Reserved


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"

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
        TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(),
            EAttachLocation::KeepWorldPosition, false);
    }
}

void AProjectileRocket::DestroyTimerFinished()
{
    Destroy();
}

void AProjectileRocket::Destroyed()
{
    
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
