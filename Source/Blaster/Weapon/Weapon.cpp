// Blaster Game. All Rights Reserved


#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
    AreaSphere->SetupAttachment(GetRootComponent());
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>("PickupWidget");
    PickupWidget->SetupAttachment(GetRootComponent());
   
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    
    if(HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }
}
