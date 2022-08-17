// Blaster Game. All Rights Reserved


#include "AmmoPickup.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (!BlasterCharacter || !BlasterCharacter->GetCombatComponent()) return;
    BlasterCharacter->GetCombatComponent()->PickupAmmo(WeaponType, AmmoAmount);

    Destroy();
}
