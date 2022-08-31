// Blaster Game. All Rights Reserved


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->InitialSpeed = InitialSpeed;
    ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!PropertyChangedEvent.Property) return;

    if (const FName PropertyName = PropertyChangedEvent.Property->GetFName();
        PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed) && ProjectileMovementComponent)
    {
        ProjectileMovementComponent->InitialSpeed = InitialSpeed;
        ProjectileMovementComponent->MaxSpeed = InitialSpeed;
    }
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (const auto OwnerController = OwnerCharacter->GetController())
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
        }
    }

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay();

    FPredictProjectilePathParams PredictParams;
    PredictParams.bTraceWithChannel = true;
    PredictParams.bTraceWithCollision = true;
    PredictParams.DrawDebugTime = 5.0f;
    PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;
    PredictParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
    PredictParams.MaxSimTime = 4.0f;
    PredictParams.ProjectileRadius = 5.0f;
    PredictParams.SimFrequency = 30.0f;
    PredictParams.StartLocation = GetActorLocation();
    PredictParams.TraceChannel = ECollisionChannel::ECC_Visibility;
    PredictParams.ActorsToIgnore.Add(this);

    FPredictProjectilePathResult PredictResult;
    UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
}
