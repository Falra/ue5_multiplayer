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
    
    UPROPERTY(Replicated)
    TArray<class ABlasterPlayerState*> TopScoringPlayers;

    void UpdateTopScore(ABlasterPlayerState* ScorePlayer);
    
private:
    float TopScore = 0.0f;
};
