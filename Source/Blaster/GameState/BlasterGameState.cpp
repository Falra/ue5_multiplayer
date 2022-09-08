// Blaster Game. All Rights Reserved


#include "BlasterGameState.h"

#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
    DOREPLIFETIME(ABlasterGameState, RedTeamScore);
    DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScorePlayer)
{
    if (TopScoringPlayers.Num() == 0)
    {
        TopScoringPlayers.Add(ScorePlayer);
        TopScore = ScorePlayer->GetScore();
    }
    else if (ScorePlayer->GetScore() == TopScore)
    {
        TopScoringPlayers.AddUnique(ScorePlayer);
    }
    else if (ScorePlayer->GetScore() > TopScore)
    {
        TopScoringPlayers.Empty();
        TopScoringPlayers.Add(ScorePlayer);
        TopScore = ScorePlayer->GetScore();
    }
}

void ABlasterGameState::OnRep_RedTeamScore()
{
    
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
    
}

void ABlasterGameState::RedTeamScores()
{
    ++RedTeamScore;
}

void ABlasterGameState::BlueTeamScores()
{
    ++BlueTeamScore;
}
