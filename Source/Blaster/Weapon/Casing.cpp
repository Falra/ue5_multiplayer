// Blaster Game. All Rights Reserved


#include "Casing.h"

ACasing::ACasing()
{
    PrimaryActorTick.bCanEverTick = false;

    CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
    SetRootComponent(CasingMesh);
    CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    CasingMesh->SetSimulatePhysics(true);
    CasingMesh->SetEnableGravity(true);
}

void ACasing::BeginPlay()
{
    Super::BeginPlay();
    CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}
