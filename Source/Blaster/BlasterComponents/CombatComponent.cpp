// Blaster Game. All Rights Reserved


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
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
        ServerFire();
    }
}

void UCombatComponent::ServerFire_Implementation()
{
    MulticastFire_Implementation();
}

void UCombatComponent::MulticastFire_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Role 1st %s"), Character->HasAuthority() ? "1" : "0");
    if (!EquippedWeapon) return;

    if (Character)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire();
        UE_LOG(LogTemp, Warning, TEXT("Role 2nd %s"), Character->HasAuthority() ? "1" : "0");
    }
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bIsAiming);
}
