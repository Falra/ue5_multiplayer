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
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
    bHealing = true;
    HealingRate = HealingTime == 0 ? HealAmount : HealAmount / HealingTime;
    AmountToHeal = HealAmount;
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

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
    InitialBaseSpeed = BaseSpeed;
    InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
    if (!Character) return;
    Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, BuffTime);

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->MaxWalkSpeed = BuffBaseSpeed;
        CharacterMovement->MaxWalkSpeedCrouched = BuffCrouchSpeed;
    }
}

void UBuffComponent::ResetSpeeds()
{
    if (!Character) return;

    if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
    {
        CharacterMovement->MaxWalkSpeed = InitialBaseSpeed;
        CharacterMovement->MaxWalkSpeedCrouched = InitialCrouchSpeed;
    }
}
