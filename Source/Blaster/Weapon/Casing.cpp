// Blaster Game. All Rights Reserved


#include "Casing.h"

ACasing::ACasing()
{
    PrimaryActorTick.bCanEverTick = false;

    CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
    SetRootComponent(CasingMesh);
}

void ACasing::BeginPlay()
{
    Super::BeginPlay();
}
