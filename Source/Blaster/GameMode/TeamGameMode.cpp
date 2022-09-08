// Blaster Game. All Rights Reserved


#include "TeamGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

ATeamGameMode::ATeamGameMode()
{
    bTeamsMatch = true;
}

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

float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
    const ABlasterPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ABlasterPlayerState>();
    const ABlasterPlayerState* VictimPlayerState = Victim->GetPlayerState<ABlasterPlayerState>();
    if (!AttackerPlayerState || !VictimPlayerState) return Super::CalculateDamage(Attacker, Victim, BaseDamage);
    if (AttackerPlayerState == VictimPlayerState) return BaseDamage;

    if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam()) return 0.0f;
    
    
    return Super::CalculateDamage(Attacker, Victim, BaseDamage);
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
