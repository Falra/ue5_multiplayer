// Blaster Game. All Rights Reserved


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
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
           // TODO: Handle restart
        }
    }
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController,
    ABlasterPlayerController* AttackerController)
{
    ABlasterPlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ABlasterPlayerState>() : nullptr;

    ABlasterPlayerState* VictimPlayerState = VictimController ? VictimController->GetPlayerState<ABlasterPlayerState>() : nullptr;

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        AttackerPlayerState->AddToScore(1.0f);
    }

    if (VictimPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        VictimPlayerState->AddToDefeats(1);
        VictimPlayerState->ShowDefeatedAnimation();
    }

    if (EliminatedCharacter)
    {
        EliminatedCharacter->Eliminate();
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
