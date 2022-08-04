// Blaster Game. All Rights Reserved


#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

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
    if (!IsHUDValid()) return;
    const float HealthPercent = Health / MaxHealth;
    BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
    const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
    BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    if (!IsHUDValid()) return;
    const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
    BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
    if (!IsHUDValid()) return;
    const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
    BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
    if (!IsHUDValid()) return;
    const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
    BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
    if (!IsHUDValid()) return;
    const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
    BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABlasterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
    if (!IsHUDValid()) return;
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
    if (!IsHUDValid()) return;
    const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
    const int32 Seconds = CountdownTime - Minutes * 60;
    const FString MatchCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdownText));
}

void ABlasterPlayerController::MulticastShowDefeatedAnimation_Implementation()
{
    if (!IsHUDValid()) return;
    BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->ShowDefeatedAnimation);
}

bool ABlasterPlayerController::IsHUDValid()
{
    BlasterHUD = !BlasterHUD ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD; 
    if (!BlasterHUD || !BlasterHUD->CharacterOverlay) return false;
    return true;
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
}

void ABlasterPlayerController::OnRep_MatchState()
{
}
