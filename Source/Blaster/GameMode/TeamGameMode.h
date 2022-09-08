// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "TeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamGameMode : public ABlasterGameMode
{
    GENERATED_BODY()

public:
    virtual void PostLogin(APlayerController* NewPlayer) override;
protected:
    virtual void HandleMatchHasStarted() override;
private:
    void AddPlayerToTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState);
};
