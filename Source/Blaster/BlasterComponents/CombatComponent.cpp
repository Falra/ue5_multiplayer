// Blaster Game. All Rights Reserved


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        Controller = Cast<ABlasterPlayerController>(Character->Controller);
        if (Controller)
        {
            HUD = Cast<ABlasterHUD>(Controller->GetHUD());
        }

        if (Character->GetFollowCamera())
        {
            DefaultFOV = Character->GetFollowCamera()->FieldOfView;
            CurrentFOV = DefaultFOV;
        }

        if (Character->HasAuthority())
        {
            InitializeCarriedAmmo();
        }
    }
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Character && Character->IsLocallyControlled())
    {
        SetHUDCrosshairs(DeltaTime);
        FHitResult HitResult;
        TraceUnderCrosshairs(HitResult);
        HitTarget = HitResult.ImpactPoint;
        InterpFOV(DeltaTime);
    }
}

void UCombatComponent::SetAiming(bool IsAiming)
{
    if (!Character || !EquippedWeapon) return;
    
    bIsAiming = IsAiming;
    ServerSetAiming(IsAiming);
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
    if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
    {
        Character->ShowSniperScopeWidget(bIsAiming);
    }
}

void UCombatComponent::ServerSetAiming_Implementation(bool IsAiming)
{
    bIsAiming = IsAiming;
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
    if (!Character) return;
    Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
    if (!Controller) return;
    HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
    if (!HUD) return;

    if (EquippedWeapon)
    {
        HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
        HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
        HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
        HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
        HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
    }
    else
    {
        HUDPackage.CrosshairsCenter = nullptr;
        HUDPackage.CrosshairsLeft = nullptr;
        HUDPackage.CrosshairsRight = nullptr;
        HUDPackage.CrosshairsTop = nullptr;
        HUDPackage.CrosshairsBottom = nullptr;
    }

    HUDPackage.CrosshairsColor = bIsAimingPlayer ? FLinearColor::Red : FLinearColor::White;
    
    const FVector2D WalkSpeedRange(0.0f, Character->GetCharacterMovement()->MaxWalkSpeed);
    const FVector2D VelocityMultiplierRange(0.0f, 1.0f);
    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0;
    CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

    if (Character->GetCharacterMovement()->IsFalling())
    {
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
    }
    else
    {
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
    }

    const float TargetAimFactor = bIsAiming ? 0.58f : 0.0f;
    CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, TargetAimFactor, DeltaTime, 30.0f);
    
    CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 40.0f);

    const float TargetAimPlayerFactor = bIsAimingPlayer ? 0.58f : 0.0f;
    CrosshairAimPlayerFactor = FMath::FInterpTo(CrosshairAimPlayerFactor, TargetAimPlayerFactor, DeltaTime, 30.0f);
    
    HUDPackage.CrosshairSpread = 0.5f
                                 + CrosshairVelocityFactor
                                 + CrosshairInAirFactor
                                 - CrosshairAimFactor
                                 + CrosshairShootingFactor
                                 - CrosshairAimPlayerFactor;

    HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
    if (!Character || !EquippedWeapon || !Character->GetFollowCamera()) return;

    if (bIsAiming)
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
    }
    else
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);
    }
    Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
    if (!GEngine || !GEngine->GameViewport) return;

    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    const FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;
    const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
        UGameplayStatics::GetPlayerController(this, 0),
        CrosshairLocation,
        CrosshairWorldPosition,
        CrosshairWorldDirection
        );

    if (!bScreenToWorld) return;
    FVector Start = CrosshairWorldPosition;
    if (Character)
    {
        const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
        Start += CrosshairWorldDirection * (DistanceToCharacter + 100.0f);
    }
    const FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
    GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

    if (!TraceHitResult.bBlockingHit)
    {
        TraceHitResult.ImpactPoint = End;
    }

    bIsAimingPlayer = TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>();
}

#pragma region EquipWeapon

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (!Character || !WeaponToEquip || CombatState != ECombatState::ECS_Unoccupied) return;

    if (EquippedWeapon)
    {
        EquippedWeapon->DropWeapon();
    }
    
    EquippedWeapon = WeaponToEquip;
    SetWeaponStateAndAttach();
    EquippedWeapon->SetOwner(Character);
    EquippedWeapon->ShowWeaponAmmo();

    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }

    Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDCarriedAmmo(CarriedAmmo);
        Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
    }

    PlayEquipEffects();

    if (EquippedWeapon->IsEmpty())
    {
        Reload();
    }
    
    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;

}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (EquippedWeapon && Character)
    {
        SetWeaponStateAndAttach();
        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
        PlayEquipEffects();
        Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
        if (Controller)
        {
            Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
        }
    }
}

void UCombatComponent::PlayEquipEffects() const
{
    if (EquippedWeapon->EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());
    }
}

void UCombatComponent::SetWeaponStateAndAttach()
{
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    if (const auto WeaponSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
    {
        WeaponSocket->AttachActor(EquippedWeapon, Character->GetMesh());
    }
}

#pragma endregion

#pragma region Fire

void UCombatComponent::Fire()
{
    if (!EquippedWeapon || !CanFire()) return;
    bCanFire = false;
    ServerFire(HitTarget);
    CrosshairShootingFactor = 0.75f;
    StartFireTimer();
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
    bFireButtonPressed = bPressed;
    if (bFireButtonPressed)
    {
        Fire();
    }
}

void UCombatComponent::ShotgunShellReload()
{
    if(Character && Character->HasAuthority())
    {
        UpdateShotgunAmmoValues();
    }
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (!EquippedWeapon) return;

    if (Character && IsEquippedShotgun() && CombatState == ECombatState::ECS_Reloading)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
        CombatState = ECombatState::ECS_Unoccupied;
    }
    else if (Character && CombatState == ECombatState::ECS_Unoccupied)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
}

void UCombatComponent::StartFireTimer()
{
    if (!EquippedWeapon || !Character) return;
    Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
    bCanFire = true;
    if (!EquippedWeapon) return;
    if (bFireButtonPressed && EquippedWeapon->bAutomatic)
    {
        Fire();
    }
    if (EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

bool UCombatComponent::CanFire() const
{
    if (!EquippedWeapon) return false;
    if (EquippedWeapon->IsEmpty() || !bCanFire) return false;
    if (IsEquippedShotgun() && CombatState == ECombatState::ECS_Reloading) return true;
    return CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
    if (Controller)
    {
        Controller->SetHUDCarriedAmmo(CarriedAmmo);
    }

    const bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading && CarriedAmmo == 0 && IsEquippedShotgun();
    if (bJumpToShotgunEnd) JumpToShotgunEnd();
}

void UCombatComponent::InitializeCarriedAmmo()
{
    CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeAmmo);

}

#pragma endregion

#pragma region Reload

void UCombatComponent::Reload()
{
    if (CarriedAmmo <= 0 || CombatState != ECombatState::ECS_Unoccupied) return;
    ServerReload();
}

void UCombatComponent::ServerReload_Implementation()
{
    if (!Character || !EquippedWeapon) return;
    CombatState = ECombatState::ECS_Reloading;
    HandleReload();
}

void UCombatComponent::HandleReload()
{
    Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
    if (!EquippedWeapon) return 0;
    const int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        const int32 Carried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
        const int32 Least = FMath::Min(RoomInMag, Carried);
        return FMath::Clamp(RoomInMag, 0, Least);
    }
    return 0;
}

void UCombatComponent::ThrowGrenade()
{
    if (CombatState != ECombatState::ECS_Unoccupied) return;
    
    CombatState = ECombatState::ECS_ThrowingGrenade;
    if (Character)
    {
        Character->PlayThrowGrenadeMontage();
    }
    if (Character && !Character->HasAuthority())
    {
        ServerThrowGrenade();
    }
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
    CombatState = ECombatState::ECS_ThrowingGrenade;
    if (Character)
    {
        Character->PlayThrowGrenadeMontage();
    }
}

void UCombatComponent::FinishReloading()
{
    if (!Character) return;

    if (Character->HasAuthority())
    {
        CombatState = ECombatState::ECS_Unoccupied;
        UpdateAmmoValues();
    }

    if (bFireButtonPressed)
    {
        Fire();
    }
}

#pragma endregion 

void UCombatComponent::OnRep_CombatState()
{
    switch (CombatState)
    {
        case ECombatState::ECS_Reloading:
            HandleReload();
            break;
        case ECombatState::ECS_Unoccupied:
            if (bFireButtonPressed)
            {
                Fire();
            }
            break;
        case ECombatState::ECS_ThrowingGrenade:
            if (Character && !Character->IsLocallyControlled())
            {
                Character->PlayThrowGrenadeMontage();
            }
            break;
    }
}

void UCombatComponent::UpdateAmmoValues()
{
    if (!EquippedWeapon) return;
    
    const int32 ReloadAmount = AmountToReload();
    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }
    Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDCarriedAmmo(CarriedAmmo);
    }
    
    EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
    if (!EquippedWeapon) return;
    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }
    Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDCarriedAmmo(CarriedAmmo);
    }
    EquippedWeapon->AddAmmo(-1);
    bCanFire = true;
    if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
    {
        JumpToShotgunEnd();
    }
}

void UCombatComponent::JumpToShotgunEnd() const
{
    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !Character->GetReloadMontage()) return;
    AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
}

void UCombatComponent::ThrowGrenadeFinished()
{
    CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bIsAiming);
    DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
    DOREPLIFETIME(UCombatComponent, CombatState);
}
