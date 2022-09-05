// Blaster Game. All Rights Reserved


#include "ReturnToMainMenu.h"

#include "Components/Button.h"

void UReturnToMainMenu::MenuSetup()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    const UWorld* World = GetWorld();
    if (!World) return;
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    FInputModeGameAndUI InputModeData;
    InputModeData.SetWidgetToFocus(TakeWidget());
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(true);
}

bool UReturnToMainMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (ReturnButton)
    {
        ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }
    return true;
}

void UReturnToMainMenu::MenuTearDown()
{
    RemoveFromParent();
    const UWorld* World = GetWorld();
    if (!World) return;
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    PlayerController->SetInputMode(FInputModeGameOnly());
    PlayerController->SetShowMouseCursor(false);
}


void UReturnToMainMenu::ReturnButtonClicked()
{
    
}
