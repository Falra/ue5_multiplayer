// Blaster Game. All Rights Reserved


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController,
    ABlasterPlayerController* AttackerController)
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Eliminate();
    }
}
