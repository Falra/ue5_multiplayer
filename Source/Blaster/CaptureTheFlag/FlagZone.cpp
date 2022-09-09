// Blaster Game. All Rights Reserved


#include "FlagZone.h"

#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"

AFlagZone::AFlagZone()
{
    PrimaryActorTick.bCanEverTick = false;

    ZoneSphere = CreateDefaultSubobject<USphereComponent>("ZoneSphere");
    SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
    Super::BeginPlay();

    ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AFlag* OverlappedFlag = Cast<AFlag>(OtherActor);
    if (!OverlappedFlag || OverlappedFlag->GetTeam() == Team) return;
    // Got enemies flag
    
    ACaptureTheFlagGameMode* FlagGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
    if (!FlagGameMode) return;

    FlagGameMode->FlagCaptured(OverlappedFlag, this);
}
