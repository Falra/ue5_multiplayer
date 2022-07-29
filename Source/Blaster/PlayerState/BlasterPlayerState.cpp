// Blaster Game. All Rights Reserved


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void ABlasterPlayerState::SetHUDScore()
{
    Character = !Character ? Cast<ABlasterCharacter>(GetPawn()) : Character;
    if (!Character) return;
    Controller = !Controller ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
    if (!Controller) return;
    Controller->SetHUDScore(GetScore());
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();
    SetHUDScore();
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    SetScore(GetScore() + ScoreAmount);
    SetHUDScore();
}
