// Blaster Game. All Rights Reserved


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
    FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>("FlagMesh");
    SetRootComponent(FlagMesh);
    GetAreaSphere()->SetupAttachment(FlagMesh);
    GetPickupWidget()->SetupAttachment(FlagMesh);

    FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
