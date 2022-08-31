// Blaster Game. All Rights Reserved


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>("RocketMovementComponent");
    RocketMovementComponent->bRotationFollowsVelocity = true;
    RocketMovementComponent->SetIsReplicated(true);
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!PropertyChangedEvent.Property) return;

    if (const FName PropertyName = PropertyChangedEvent.Property->GetFName();
        PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed) && RocketMovementComponent)
    {
        RocketMovementComponent->InitialSpeed = InitialSpeed;
        RocketMovementComponent->MaxSpeed = InitialSpeed;
    }
}
#endif

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
    }

    SpawnTrailSystem();
    
    if (ProjectileLoop && LoopingSoundAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), FName(), GetActorLocation(),
            GetActorRotation(), EAttachLocation::KeepWorldPosition, false, 1.0f, 1.0f, 0.0f, LoopingSoundAttenuation,
            (USoundConcurrency*)nullptr, false);
    }
}

void AProjectileRocket::HideAndStopRocket() const
{
    if (ProjectileMesh)
    {
        ProjectileMesh->SetVisibility(false);
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
        ProjectileLoopComponent->Stop();
    }
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (OtherActor == GetOwner())
    {
        return;
    }
    
    CheckIfHitPlayer(OtherActor);
    ExplodeDamage();
    SpawnDestroyEffects();
    HideAndStopRocket();
    StartDestroyTimer();
}

void AProjectileRocket::Destroyed()
{
    // Do nothing because all effects was already spawned
}
