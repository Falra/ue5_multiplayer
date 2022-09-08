// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void UpdateTopScore(class ABlasterPlayerState* ScorePlayer);

    UPROPERTY(Replicated)
    TArray<ABlasterPlayerState*> TopScoringPlayers;

#pragma region Teams

    UPROPERTY()
    TArray<ABlasterPlayerState*> RedTeam;
    
    UPROPERTY()
    TArray<ABlasterPlayerState*> BlueTeam;

    UPROPERTY(ReplicatedUsing = "OnRep_RedTeamScore")
    float RedTeamScore = 0.0f;

    UPROPERTY(ReplicatedUsing = "OnRep_BlueTeamScore")
    float BlueTeamScore = 0.0f;

    UFUNCTION()
    void OnRep_RedTeamScore();

    UFUNCTION()
    void OnRep_BlueTeamScore();

    void RedTeamScores();

    void BlueTeamScores();

#pragma endregion 

private:
    float TopScore = 0.0f;
};
