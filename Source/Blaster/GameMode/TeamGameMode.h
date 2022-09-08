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
    ATeamGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
    virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimController,
            ABlasterPlayerController* AttackerController) override;
protected:
    virtual void HandleMatchHasStarted() override;
private:
    static void AddPlayerToTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState);
    static void RemovePlayerFromTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState);
    
};
