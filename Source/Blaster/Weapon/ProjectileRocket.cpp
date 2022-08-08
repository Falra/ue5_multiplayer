// Blaster Game. All Rights Reserved


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (const APawn* FiringPawn = GetInstigator())
    {
        if (AController* FiringController = FiringPawn->GetController())
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, RocketMinimumDamage, GetActorLocation(), RocketDamageInnerRadius,
                RocketDamageOuterRadius, 1.0f, UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController);
        }
    }
    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
