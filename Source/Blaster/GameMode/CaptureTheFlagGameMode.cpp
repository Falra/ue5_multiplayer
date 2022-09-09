// Blaster Game. All Rights Reserved


#include "CaptureTheFlagGameMode.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController,
    ABlasterPlayerController* AttackerController)
{
    ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}
