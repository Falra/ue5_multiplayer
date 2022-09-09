// Blaster Game. All Rights Reserved


#include "Weapon.h"

#include "Casing.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicatingMovement(true);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
    WeaponMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);
    
    AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
    AreaSphere->SetupAttachment(GetRootComponent());
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>("PickupWidget");
    PickupWidget->SetupAttachment(GetRootComponent());
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, WeaponState);
    DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();

    if (!Owner)
    {
        BlasterOwnerCharacter = nullptr;
        BlasterOwnerController = nullptr;
    }
    else
    {
        CheckUpdateController();
        if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent()
            && BlasterOwnerCharacter->GetCombatComponent()->GetEquippedWeapon() == this)
        {
            ShowWeaponAmmo();
        }
    }

}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    ShowPickupWidget(false);

    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
    AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
    {
        if (WeaponType == EWeaponType::EWT_Flag && Team != BlasterCharacter->GetTeam()) return;
        
        if (BlasterCharacter->IsHoldingTheFlag()) return;

        BlasterCharacter->SetOverlappingWeapon(this);
    }
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (BlasterCharacter)
    {
        if (WeaponType == EWeaponType::EWT_Flag && Team != BlasterCharacter->GetTeam()) return;

        if (BlasterCharacter->IsHoldingTheFlag()) return;
        
        BlasterCharacter->SetOverlappingWeapon(nullptr);
    }
}

void AWeapon::SetWeaponState(EWeaponState State)
{
    WeaponState = State;

    OnSetWeaponState();
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
    bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
    OnSetWeaponState();
}

void AWeapon::OnSetWeaponState()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equipped:
            OnEquipped();
            break;
        case EWeaponState::EWS_EquippedSecondary:
            OnEquippedSecondary();
            break;
        case EWeaponState::EWS_Dropped:
            OnDropped();
            break;
    }
}

void AWeapon::OnEquipped()
{
    ShowPickupWidget(false);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetWeaponMeshState(false);

    BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    BlasterOwnerController = (!BlasterOwnerController && BlasterOwnerCharacter) ? BlasterOwnerCharacter->GetController<ABlasterPlayerController>() : BlasterOwnerController;
    if (BlasterOwnerController && bUseServerSideRewind && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
    {
        BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
    }
}

void AWeapon::OnDropped()
{
    if (HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    SetWeaponMeshState(true);

    BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    BlasterOwnerController = (!BlasterOwnerController && BlasterOwnerCharacter) ? BlasterOwnerCharacter->GetController<ABlasterPlayerController>() : BlasterOwnerController;
    if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
    {
        BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
    }
}

void AWeapon::OnEquippedSecondary()
{
    ShowPickupWidget(false);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetWeaponMeshState(false, true);

    BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    BlasterOwnerController = (!BlasterOwnerController && BlasterOwnerCharacter) ? BlasterOwnerCharacter->GetController<ABlasterPlayerController>() : BlasterOwnerController;
    if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
    {
        BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
    }
}

void AWeapon::CheckUpdateController()
{
    BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    BlasterOwnerController = (!BlasterOwnerController && BlasterOwnerCharacter) ? BlasterOwnerCharacter->GetController<ABlasterPlayerController>() : BlasterOwnerController;
}

void AWeapon::SpendRound()
{
    Ammo = FMath::Clamp(Ammo - 1, 0 ,MagCapacity);

    ShowWeaponAmmo();

    if (HasAuthority())
    {
        ClientUpdateAmmo(Ammo);
    }
    else
    {
        ++AmmoSequence;
    }
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
    if (HasAuthority()) return;
    
    Ammo = ServerAmmo;
    --AmmoSequence;
    Ammo -= AmmoSequence;

    ShowWeaponAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
    ShowWeaponAmmo();
    ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
    if (HasAuthority()) return;
    
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
    BlasterOwnerCharacter = !BlasterOwnerCharacter ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
    if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent() && IsFull())
    {
        BlasterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
    }
    ShowWeaponAmmo();
}

void AWeapon::ShowWeaponAmmo()
{
    CheckUpdateController();

    if (BlasterOwnerController)
    {
        BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
    }
}

bool AWeapon::IsEmpty() const
{
    return Ammo <= 0;
}

bool AWeapon::IsFull() const
{
    return Ammo == MagCapacity;
}

void AWeapon::SetWeaponMeshState(bool bIsEnabled, bool bIsSecondary)
{
    WeaponMesh->SetSimulatePhysics(bIsEnabled);
    WeaponMesh->SetEnableGravity(bIsEnabled);
    const auto CollisionEnabled = bIsEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
    WeaponMesh->SetCollisionEnabled(CollisionEnabled);
    EnableCustomDepth(bIsEnabled || bIsSecondary);
    if (bIsEnabled)
    {
        WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
        WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
        WeaponMesh->MarkRenderStateDirty();
    }
    else if (bIsSecondary)
    {
        WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
        WeaponMesh->MarkRenderStateDirty();
    }
    else if (bHasStrap)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WeaponMesh->SetEnableGravity(true);
        WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
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
    SpendRound();
}

void AWeapon::DropWeapon()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
    WeaponMesh->DetachFromComponent(TransformRules);
    SetOwner(nullptr);
    BlasterOwnerCharacter = nullptr;
    BlasterOwnerController = nullptr;
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
    if (WeaponMesh)
    {
        WeaponMesh->SetRenderCustomDepth(bEnable);
    }
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget) const
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (!MuzzleFlashSocket) return FVector();

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.0f, SphereRadius);
    const FVector EndLoc = SphereCenter + RandVector;
    const FVector ToEndLoc = EndLoc - TraceStart;
    const FVector Result = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
    
    return Result;
}