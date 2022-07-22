// Blaster Game. All Rights Reserved


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
    }
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SetHUDCrosshairs(DeltaTime);

    if (Character && Character->IsLocallyControlled())
    {
        FHitResult HitResult;
        TraceUnderCrosshairs(HitResult);
        HitTarget = HitResult.ImpactPoint;
    }
}

void UCombatComponent::SetAiming(bool IsAiming)
{
    bIsAiming = IsAiming;
    ServerSetAiming(IsAiming);
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (!Character || !WeaponToEquip) return;

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    if (const auto WeaponSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
    {
        WeaponSocket->AttachActor(EquippedWeapon, Character->GetMesh());
    }
    EquippedWeapon->SetOwner(Character);

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (EquippedWeapon && Character)
    {
        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
    }
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
    bFireButtonPressed = bPressed;
    if (bFireButtonPressed)
    {
        FHitResult HitResult;
        TraceUnderCrosshairs(HitResult);
        ServerFire(HitResult.ImpactPoint);
    }
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (!EquippedWeapon) return;

    if (Character)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
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
    const FVector Start = CrosshairWorldPosition;
    const FVector End = Start + CrosshairWorldDirection * CrosshairTraceLenght;
    GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

    if (!TraceHitResult.bBlockingHit)
    {
        TraceHitResult.ImpactPoint = End;
    }
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
    if (!Character || !Controller || !HUD) return;

    FHUDPackage HUDPackage;
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

    HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;

    HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bIsAiming);
}
