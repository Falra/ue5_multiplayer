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

void ATeamGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
    if (!BlasterGameState) return;

    ABlasterPlayerState* BlasterPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();
    if (!BlasterPlayerState) return;

    RemovePlayerFromTeam(BlasterGameState, BlasterPlayerState);
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

void ATeamGameMode::RemovePlayerFromTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState)
{
    if (BlasterGameState->RedTeam.Contains(BlasterPlayerState))
    {
        BlasterGameState->RedTeam.Remove(BlasterPlayerState);
    }

    if (BlasterGameState->BlueTeam.Contains(BlasterPlayerState))
    {
        BlasterGameState->BlueTeam.Remove(BlasterPlayerState);
    }
}
