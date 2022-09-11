// Blaster Game. All Rights Reserved

#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    const auto NumberOfPlayers = GameState.Get()->PlayerArray.Num();
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance) return;
    UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    if (!Subsystem) return;
    
    if(NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
    {
        if(const auto World = GetWorld())
        {
            bUseSeamlessTravel = true;
            const FString MatchType = Subsystem->DesiredMatchType;
            if (MatchType == "FreeForAll")
            {
                World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
            }
            else if (MatchType == "Teams")
            {
                World->ServerTravel(FString("/Game/Maps/Teams?listen"));
            }
            else if (MatchType == "CaptureTheFlag")
            {
                World->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
            }
        }
    }
}
