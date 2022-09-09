// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACaptureTheFlagGameMode : public ATeamGameMode
{
    GENERATED_BODY()
public:
    virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimController,
            ABlasterPlayerController* AttackerController) override;
};
