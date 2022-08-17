// Blaster Game. All Rights Reserved


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"

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
