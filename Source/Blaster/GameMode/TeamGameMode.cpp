// Blaster Game. All Rights Reserved


#include "TeamGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
    if (!BlasterGameState) return;

    ABlasterPlayerState* BlasterPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
    if (!BlasterPlayerState) return;

    AddPlayerToTeam(BlasterGameState, BlasterPlayerState);
}

void ATeamGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
    if (!BlasterGameState) return;

    for (auto PState: BlasterGameState->PlayerArray)
    {
        ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PState.Get());
        if (!BlasterPlayerState) continue;
        AddPlayerToTeam(BlasterGameState, BlasterPlayerState);
    }
}

void ATeamGameMode::AddPlayerToTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState)
{
    if (BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
    {
        if (BlasterGameState->BlueTeam.Num() >= BlasterGameState->RedTeam.Num())
        {
            BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
            BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
        }
        else
        {
            BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
            BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
        }
    }
}
