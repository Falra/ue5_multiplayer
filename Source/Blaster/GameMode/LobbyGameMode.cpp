// Blaster Game. All Rights Reserved

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    const auto NumberOfPlayers = GameState.Get()->PlayerArray.Num();
    if(NumberOfPlayers == 2)
    {
        if(const auto World = GetWorld())
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(FString("/Game/Maps/BlasterMapNew?listen"));
        }
    }
}
