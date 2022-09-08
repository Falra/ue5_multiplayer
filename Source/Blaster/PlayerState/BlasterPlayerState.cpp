// Blaster Game. All Rights Reserved


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterPlayerState, Defeats);
    DOREPLIFETIME(ABlasterPlayerState, Team);
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();
    SetHUDScore();
}

void ABlasterPlayerState::OnRep_Defeats()
{
    SetHUDDefeats();
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    SetScore(GetScore() + ScoreAmount);
    SetHUDScore();
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
    Defeats += DefeatsAmount;
    SetHUDDefeats();
}

void ABlasterPlayerState::SetHUDScore()
{
    CheckSetController();
    if (!Controller) return;
    Controller->SetHUDScore(GetScore());
}

void ABlasterPlayerState::SetHUDDefeats()
{
    CheckSetController();
    if (!Controller) return;
    Controller->SetHUDDefeats(Defeats);
}

void ABlasterPlayerState::ShowDefeatedAnimation()
{
    CheckSetController();
    if (!Controller) return;
    Controller->MulticastShowDefeatedAnimation();
}

void ABlasterPlayerState::CheckSetController()
{
    Character = !Character ? Cast<ABlasterCharacter>(GetPawn()) : Character;
    if (!Character) return;
    Controller = !Controller ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
}
