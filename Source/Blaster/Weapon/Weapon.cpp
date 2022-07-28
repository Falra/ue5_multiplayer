// Blaster Game. All Rights Reserved


#include "Weapon.h"

#include "Casing.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

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

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    ShowPickupWidget(false);

    if (HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
        AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
    }
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (BlasterCharacter)
    {
        BlasterCharacter->SetOverlappingWeapon(this);
    }
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (BlasterCharacter)
    {
        BlasterCharacter->SetOverlappingWeapon(nullptr);
    }
}

void AWeapon::SetWeaponState(EWeaponState State)
{
    WeaponState = State;

    switch (WeaponState)
    {
        case EWeaponState::EWS_Equipped:
            ShowPickupWidget(false);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SetWeaponMeshState(false);
            break;
        case EWeaponState::EWS_Dropped:
            if (HasAuthority())
            {
                AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            SetWeaponMeshState(true);
            break;
    }
}

void AWeapon::OnRep_WeaponState()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equipped:
            ShowPickupWidget(false);
            SetWeaponMeshState(false);
            break;
        case EWeaponState::EWS_Dropped:
            SetWeaponMeshState(true);
            break;
    }
}

void AWeapon::SetWeaponMeshState(bool bIsEnabled)
{
    WeaponMesh->SetSimulatePhysics(bIsEnabled);
    WeaponMesh->SetEnableGravity(bIsEnabled);
    const auto CollisionEnabled = bIsEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
    WeaponMesh->SetCollisionEnabled(CollisionEnabled);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::Fire(const FVector& HitTarget)
{
    if (FireAnimation)
    {
        WeaponMesh->PlayAnimation(FireAnimation, false);
    }
    if (CasingClass)
    {
        const auto AmmoSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
        if (!AmmoSocket) return;
        const auto SocketTransform = AmmoSocket->GetSocketTransform(WeaponMesh);
        UWorld* World = GetWorld();
        if (!World) return;
        FRotator ShellRotation = SocketTransform.GetRotation().Rotator();

        ShellRotation.Pitch += FMath::RandRange(ShellOffset.X, ShellOffset.Y);
        ShellRotation.Roll += FMath::RandRange(ShellOffset.X, ShellOffset.Y);
        ShellRotation.Yaw += FMath::RandRange(ShellOffset.X, ShellOffset.Y);

        World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), ShellRotation);
    }
}

void AWeapon::DropWeapon()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
    WeaponMesh->DetachFromComponent(TransformRules);
    SetOwner(nullptr);
}
