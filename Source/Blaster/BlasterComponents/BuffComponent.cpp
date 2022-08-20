// Blaster Game. All Rights Reserved


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    HealRampUp(DeltaTime);
    ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
    bHealing = true;
    HealingRate = HealingTime == 0 ? HealAmount : HealAmount / HealingTime;
    AmountToHeal = HealAmount;
}

void UBuffComponent::BuffShield(float ShieldAmount, float BuffTime)
{
    bShieldBuff = true;
    ShieldRate = BuffTime == 0 ? ShieldAmount : ShieldAmount / BuffTime;
    ShieldAmountToAdd = ShieldAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
    if (!bHealing || !Character || Character->IsEliminated()) return;
    const float HealThisFrame = HealingRate == AmountToHeal ? AmountToHeal : HealingRate * DeltaTime;
    Character->SetHealth(Character->GetHealth() + HealThisFrame);
    AmountToHeal -= HealThisFrame;
    if (AmountToHeal <= 0.0f)
    {
        bHealing = false;
    }
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
    if (!bShieldBuff || !Character || Character->IsEliminated()) return;
    const float ShieldThisFrame = ShieldRate == ShieldAmountToAdd ? ShieldAmountToAdd : ShieldRate * DeltaTime;
    Character->SetShield(Character->GetShield() + ShieldThisFrame);
    ShieldAmountToAdd -= ShieldThisFrame;
    if (ShieldAmountToAdd <= 0.0f)
    {
        bShieldBuff = false;
    }
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
    InitialBaseSpeed = BaseSpeed;
    InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpSpeed(float JumpSpeed)
{
    InitialJumpSpeed = JumpSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
    if (!Character) return;
    Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, BuffTime);

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->MaxWalkSpeed = BuffBaseSpeed;
        CharacterMovement->MaxWalkSpeedCrouched = BuffCrouchSpeed;
        MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
    }
}

void UBuffComponent::ResetSpeeds()
{
    if (!Character) return;

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->MaxWalkSpeed = InitialBaseSpeed;
        CharacterMovement->MaxWalkSpeedCrouched = InitialCrouchSpeed;
        MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
    }
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->MaxWalkSpeed = BaseSpeed;
        CharacterMovement->MaxWalkSpeedCrouched = CrouchSpeed;
    }
}

void UBuffComponent::BuffJump(float BuffJumpSpeed, float BuffTime)
{
    if (!Character) return;
    Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJumpSpeed, BuffTime);

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->JumpZVelocity = BuffJumpSpeed;
        MulticastJumpBuff(BuffJumpSpeed);
    }
}

void UBuffComponent::ResetJumpSpeed()
{
    if (!Character) return;

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->JumpZVelocity = InitialJumpSpeed;
        MulticastJumpBuff(InitialJumpSpeed);
    }
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpSpeed)
{
    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->JumpZVelocity = JumpSpeed;
    }
}
