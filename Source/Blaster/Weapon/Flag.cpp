// Blaster Game. All Rights Reserved


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
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

void AFlag::DropWeapon()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
    FlagMesh->DetachFromComponent(TransformRules);
    SetOwner(nullptr);
    BlasterOwnerCharacter = nullptr;
    BlasterOwnerController = nullptr;
}

void AFlag::ResetFlag()
{
    if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwner()))
    {
        BlasterCharacter->SetHoldingTheFlag(false);
    }
    
    const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
    FlagMesh->DetachFromComponent(TransformRules);
    SetOwner(nullptr);
    BlasterOwnerCharacter = nullptr;
    BlasterOwnerController = nullptr;

    SetActorTransform(InitialTransform);
}

void AFlag::BeginPlay()
{
    Super::BeginPlay();
    
    InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
    ShowPickupWidget(false);
    GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FlagMesh->SetSimulatePhysics(false);
    FlagMesh->SetEnableGravity(false);
    FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
    EnableCustomDepth(false);
}

void AFlag::OnDropped()
{
    if (HasAuthority())
    {
        GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    FlagMesh->SetSimulatePhysics(true);
    FlagMesh->SetEnableGravity(true);
    FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    FlagMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);
}
