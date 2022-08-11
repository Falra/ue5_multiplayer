// Blaster Game. All Rights Reserved


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Blaster.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
    SetRootComponent(CollisionBox);
    CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(3.0f);
    
    if (Tracer)
    {
        TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
    }

    if (HasAuthority())
    {
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    }
}

void AProjectile::CheckIfHitPlayer(AActor* OtherActor)
{
    if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
    {
        bHitPlayer = true;
    }
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    CheckIfHitPlayer(OtherActor);
    Destroy();
}

void AProjectile::SpawnTrailSystem()
{
    if (TrailSystem)
    {
        TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition, false);
    }
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectile::SpawnDestroyEffects() const
{
    if (bHitPlayer && ImpactPlayerParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerParticles, GetActorTransform());
    }
    else if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
}

void AProjectile::Destroyed()
{
    Super::Destroyed();

    SpawnDestroyEffects();
}
