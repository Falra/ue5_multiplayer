// Blaster Game. All Rights Reserved


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(const FString TextToDisplay) const
{
    if (!DisplayText) return;

    DisplayText->SetText(FText::FromString(TextToDisplay));
}

void UOverheadWidget::ShowPlayerNetRole(const APawn* InPawn) const
{
    const auto LocalRole = InPawn->GetLocalRole();
    FString Role;
    switch (LocalRole)
    {
        case ENetRole::ROLE_Authority: Role = FString("Authority");
            break;
        case ENetRole::ROLE_AutonomousProxy: Role = FString("Autonomous proxy");
            break;
        case ENetRole::ROLE_SimulatedProxy: Role = FString("Simulated proxy");
            break;
        case ENetRole::ROLE_None: Role = FString("None");
            break;
        default: Role = FString("None");
    }
    const FString LocalRoleString = FString::Printf(TEXT("Local role: %s"), *Role);
    SetDisplayText(LocalRoleString);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    RemoveFromParent();
    Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
