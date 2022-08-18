// Blaster Game. All Rights Reserved


#include "SpeedPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (!BlasterCharacter || !BlasterCharacter->GetBuffComponent()) return;
    BlasterCharacter->GetBuffComponent()->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
    Destroy();
}
