// Blaster Game. All Rights Reserved


#include "CaptureTheFlagGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/Weapon/Flag.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController,
    ABlasterPlayerController* AttackerController)
{
    ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
    if (!Flag || !FlagZone || Flag->GetTeam() == FlagZone->Team) return;
    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
    if (!BlasterGameState) return;

    if (FlagZone->Team == ETeam::ET_RedTeam)
    {
        BlasterGameState->RedTeamScores();
    }
    else if (FlagZone->Team == ETeam::ET_BlueTeam)
    {
        BlasterGameState->BlueTeamScores();
    }
}
