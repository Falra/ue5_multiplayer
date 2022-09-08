// Blaster Game. All Rights Reserved


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
    const FName Cooldown = FName(TEXT("Cooldown"));
}

ABlasterGameMode::ABlasterGameMode()
{
    bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
    Super::BeginPlay();
    CountdownTime = WarmupTime;
    LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(It->Get());
        if (!BlasterPlayer) continue;
        BlasterPlayer->OnMatchStateSet(MatchState);
    }
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (MatchState == MatchState::WaitingToStart)
    {
        CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.0f)
        {
            StartMatch();
            CountdownTime = WarmupTime + MatchTime;
        }
    }
    else if (MatchState == MatchState::InProgress)
    {
        CountdownTime = MatchTime + WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.0f)
        {
            SetMatchState(MatchState::Cooldown);
        }
    }
    else if (MatchState == MatchState::Cooldown)
    {
        CountdownTime = CooldownTime + MatchTime + WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.0f)
        {
           RestartGame();
        }
    }
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController,
    ABlasterPlayerController* AttackerController)
{
    ABlasterPlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ABlasterPlayerState>() : nullptr;
    ABlasterPlayerState* VictimPlayerState = VictimController ? VictimController->GetPlayerState<ABlasterPlayerState>() : nullptr;
    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
    {
        TArray<ABlasterPlayerState*> CurrentLeads;
        for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
        {
            CurrentLeads.Add(LeadPlayer);
        }
        
        AttackerPlayerState->AddToScore(1.0f);
        BlasterGameState->UpdateTopScore(AttackerPlayerState);

        if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
        {
            if (ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn()))
            {
                Leader->MulticastGainTheLead();
            }
        }
        
        for (int32 i = 0; i < CurrentLeads.Num(); ++i)
        {
            if (!BlasterGameState->TopScoringPlayers.Contains(CurrentLeads[i]))
            {
                if (ABlasterCharacter* Loser = Cast<ABlasterCharacter>(CurrentLeads[i]->GetPawn()))
                {
                    Loser->MulticastLostTheLead();
                }
            }
        }
    }

    if (VictimPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        VictimPlayerState->AddToDefeats(1);
        VictimPlayerState->ShowDefeatedAnimation();
    }

    if (EliminatedCharacter)
    {
        EliminatedCharacter->Eliminate(false);
    }

    if (AttackerPlayerState && VictimPlayerState)
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            const auto BlasterPlayer = Cast<ABlasterPlayerController>(It->Get());
            if (!BlasterPlayer) continue;
            BlasterPlayer->BroadcastElimination(AttackerPlayerState, VictimPlayerState);
        }
    }
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Reset();
        EliminatedCharacter->Destroy();
    }
    if (EliminatedController)
    {
        TArray<AActor*> PlayerStarts; 
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
        const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
        RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
    }
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
    if (!PlayerLeaving) return;

    ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
    if (!BlasterGameState) return;

    if (BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
    {
        BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
    }
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
    if (!BlasterCharacter) return;
    BlasterCharacter->Eliminate(true);
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
    return BaseDamage;
}
