// Blaster Game. All Rights Reserved


#include "HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"

AHealthPickup::AHealthPickup()
{
    bReplicates = true;

    PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>("PickupEffectComponent");
    PickupEffectComponent->SetupAttachment(RootComponent);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (!BlasterCharacter || !BlasterCharacter->GetBuffComponent()) return;
    
    Destroy();
}

void AHealthPickup::Destroyed()
{
    if (PickupEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
    }
    Super::Destroyed();
}
