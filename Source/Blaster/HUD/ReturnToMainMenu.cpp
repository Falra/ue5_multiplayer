// Blaster Game. All Rights Reserved


#include "ReturnToMainMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::MenuSetup()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    if (const UWorld* World = GetWorld())
    {
        PlayerController = !PlayerController ? World->GetFirstPlayerController() : PlayerController;
        if (PlayerController)
        {
            FInputModeGameAndUI InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    if (ReturnButton && !ReturnButton->OnClicked.IsBound())
    {
        ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }

    if (const UGameInstance* GameInstance = GetGameInstance())
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        if (MultiplayerSessionsSubsystem)
        {
            MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
        }
    }
}

bool UReturnToMainMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }
    
    return true;
}

void UReturnToMainMenu::MenuTearDown()
{
    RemoveFromParent();
    if (const UWorld* World = GetWorld())
    {
        PlayerController = !PlayerController ? World->GetFirstPlayerController() : PlayerController;
        if (PlayerController)
        {
            PlayerController->SetInputMode(FInputModeGameOnly());
            PlayerController->SetShowMouseCursor(false);
        }
    }

    if (ReturnButton && ReturnButton->OnClicked.IsBound())
    {
        ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
    }
}

void UReturnToMainMenu::ReturnButtonClicked()
{
    ReturnButton->SetIsEnabled(false);
    
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();
    }
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        ReturnButton->SetIsEnabled(true);
        return;
    }
    
    if (const UWorld* World = GetWorld())
    {
        if (AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
        {
            GameMode->ReturnToMainMenuHost();
        }
        else
        {
            PlayerController = !PlayerController ? World->GetFirstPlayerController() : PlayerController;
            if (!PlayerController) return;
            PlayerController->ClientReturnToMainMenuWithTextReason(FText());
        }
    }
}
