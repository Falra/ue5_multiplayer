// Blaster Game. All Rights Reserved


#include "BlasterPlayerController.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());

    ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    SetHUDTime();

    CheckTimeSync(DeltaSeconds);

    PollInit();
}

void ABlasterPlayerController::CheckTimeSync(float DeltaSeconds)
{
    if (!IsLocalController()) return;
    TimeSyncRunningTime += DeltaSeconds;
    if (TimeSyncRunningTime > TimeSyncFrequency)
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
        TimeSyncRunningTime = 0.0f;
    }
}

void ABlasterPlayerController::PollInit()
{
    if (!CharacterOverlay)
    {
        if (BlasterHUD && BlasterHUD->CharacterOverlay)
        {
            CharacterOverlay = BlasterHUD->CharacterOverlay;
            if (CharacterOverlay)
            {
                SetHUDHealth(HUDHealth, HUDMaxHealth);
                SetHUDShield(HUDShield, HUDMaxShield);
                SetHUDScore(HUDScore);
                SetHUDDefeats(HUDDefeats);

                if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
                    BlasterCharacter && BlasterCharacter->GetCombatComponent())
                {
                    HUDGrenades = BlasterCharacter->GetCombatComponent()->GetGrenades();
                }
                SetHUDGrenades(HUDGrenades);
            }
        }
    }
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
    const ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GameMode) return;
    WarmupTime = GameMode->WarmupTime;
    MatchTime = GameMode->MatchTime;
    CooldownTime = GameMode->CooldownTime;
    LevelStartingTime = GameMode->LevelStartingTime;
    MatchState = GameMode->GetMatchState();
    ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

    if (BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncementWidget();
    }
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName State, float Warmup, float Match, float Cooldown, float StartingTime)
{
    WarmupTime = Warmup;
    MatchTime = Match;
    CooldownTime = Cooldown;
    LevelStartingTime = StartingTime;
    MatchState = State;
    OnMatchStateSet(MatchState);

    if (BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncementWidget();
    }
}

void ABlasterPlayerController::OnPossess(APawn* aPawn)
{
    Super::OnPossess(aPawn);
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(aPawn))
    {
        SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
    }
}

void ABlasterPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    if (IsLocalController())
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
    }
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay)
    {
        bInitialiseCharacterOverlay = true;
        HUDHealth = Health;
        HUDMaxHealth = MaxHealth;
        return;
    }
    const float HealthPercent = Health / MaxHealth;
    BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
    const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
    BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay)
    {
        bInitialiseCharacterOverlay = true;
        HUDShield = Shield;
        HUDMaxShield = MaxShield;
        return;
    }
    const float ShieldPercent = Shield / MaxShield;
    BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
    const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
    BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay)
    {
        bInitialiseCharacterOverlay = true;
        HUDScore = Score;
        return;
    }
    const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
    BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay)
    {
        bInitialiseCharacterOverlay = true;
        HUDDefeats = Defeats;
        return;
    }
    const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
    BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay) return;
    const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
    BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay) return;
    const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
    BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABlasterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay) return;
    FString WeaponTypeText;
    switch (WeaponType)
    {
        case EWeaponType::EWT_AssaultRifle:
            WeaponTypeText = "Assault Rifle";
            break;
    }
    BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeaponTypeText));
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay) return;
    if (CountdownTime <= 0.0f)
    {
        BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
        return;
    }
    const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
    const int32 Seconds = CountdownTime - Minutes * 60;
    const FString MatchCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdownText));
    if (CountdownTime <= WarningTimeThreshold)
    {
        BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->BlinkCountdown);
    }
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
    if (!IsHUDValid() || !BlasterHUD->AnnouncementWidget) return;
    if (CountdownTime <= 0.0f)
    {
        BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText());
        return;
    }
    const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
    const int32 Seconds = CountdownTime - Minutes * 60;
    const FString WarmupCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(WarmupCountdownText));
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay)
    {
        bInitialiseCharacterOverlay = true;
        HUDGrenades = Grenades;
        return;
    }
    const FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
    BlasterHUD->CharacterOverlay->GrenadesAmount->SetText(FText::FromString(GrenadesText));
}

void ABlasterPlayerController::MulticastShowDefeatedAnimation_Implementation()
{
    if (!IsHUDValid() || !BlasterHUD->CharacterOverlay) return;
    BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->ShowDefeatedAnimation);
}

bool ABlasterPlayerController::IsHUDValid()
{
    BlasterHUD = !BlasterHUD ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD; 
    return BlasterHUD != nullptr;
}

void ABlasterPlayerController::SetHUDTime()
{
    float TimeLeft = 0.0f;

    if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
    else if (MatchState == MatchState::InProgress)  TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
    else if (MatchState == MatchState::Cooldown)  TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;

    if (HasAuthority())
    {
        BlasterGameMode = !BlasterGameMode ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
        // BlasterGameMode = !BlasterGameMode ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
        if (BlasterGameMode)
        {
            TimeLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
        }
    }
    const uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
    if (SecondsLeft == CountdownInt) return;

    CountdownInt = SecondsLeft;
    if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
    {
        SetHUDAnnouncementCountdown(CountdownInt);
    }
    else if (MatchState == MatchState::InProgress)
    {
        SetHUDMatchCountdown(CountdownInt);
    }
    
}

float ABlasterPlayerController::GetServerTime()
{
    if (HasAuthority()) return GetWorld()->GetTimeSeconds();
    
    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
    const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
    const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    const float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
    MatchState = State;
    HandleStateChange();
}

void ABlasterPlayerController::OnRep_MatchState()
{
    HandleStateChange();
}

void ABlasterPlayerController::HandleStateChange()
{
    if (MatchState == MatchState::InProgress)
    {
        HandleMatchHasStarted();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleCooldown();
    }
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
    if (!IsHUDValid()) return;
    BlasterHUD->AddCharacterOverlay();
    if (BlasterHUD->AnnouncementWidget)
    {
        BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ABlasterPlayerController::HandleCooldown()
{
    SetCharacterInputEnabled(false);
    if (!IsHUDValid()) return;
    if (BlasterHUD->CharacterOverlay)
    {
        BlasterHUD->CharacterOverlay->RemoveFromParent();
    }
    if (BlasterHUD->AnnouncementWidget)
    {
        BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
        const FString AnnouncementText = TEXT("New Match starts in:");
        BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));

        FString InfoTextString;
            
        // ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
        if (const ABlasterGameState* BlasterGameState = GetWorld()->GetGameState<ABlasterGameState>())
        {
            const ABlasterPlayerState* ThisPlayerState = GetPlayerState<ABlasterPlayerState>();
            const TArray<ABlasterPlayerState*> TopScoringPlayers = BlasterGameState->TopScoringPlayers;
            if (TopScoringPlayers.Num() == 0)
            {
                InfoTextString = FString("There is no winner :(");
            }
            else if (TopScoringPlayers.Num() == 1 && TopScoringPlayers[0] == ThisPlayerState)
            {
                InfoTextString = FString("You are the winner !!!");
            }
            else if (TopScoringPlayers.Num() == 1)
            {
                InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopScoringPlayers[0]->GetPlayerName());
            }
            else // More than one winner
            {
                InfoTextString = FString("Winners are:\n");
                for (const auto Winner : TopScoringPlayers)
                {
                    InfoTextString.Append(FString::Printf(TEXT("%s\n"), *Winner->GetPlayerName()));
                }
            }
        }
        BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoTextString));
    }
}

void ABlasterPlayerController::SetCharacterInputEnabled(bool bIsEnabled)
{
    if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
    {
        BlasterCharacter->bDisableGameplay = !bIsEnabled;
        if (!bIsEnabled) return;
        if (UCombatComponent* CombatComponent = BlasterCharacter->GetCombatComponent())
        {
            CombatComponent->FireButtonPressed(false);
        }
    }
}
