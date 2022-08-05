// Blaster Game. All Rights Reserved


#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ServerCheckMatchState();
    
    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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
                SetHUDScore(HUDScore);
                SetHUDDefeats(HUDDefeats);
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
    LevelStartingTime = GameMode->LevelStartingTime;
    MatchState = GameMode->GetMatchState();
    ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

    if (BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncementWidget();
    }
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName State, float Warmup, float Match, float StartingTime)
{
    WarmupTime = Warmup;
    MatchTime = Match;
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
    const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
    const int32 Seconds = CountdownTime - Minutes * 60;
    const FString MatchCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdownText));
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
    const uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
    if (SecondsLeft == CountdownInt) return;

    CountdownInt = SecondsLeft;
    SetHUDMatchCountdown(CountdownInt);
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
        if (!IsHUDValid()) return;
        BlasterHUD->AddCharacterOverlay();
        if (BlasterHUD->AnnouncementWidget)
        {
            BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}
