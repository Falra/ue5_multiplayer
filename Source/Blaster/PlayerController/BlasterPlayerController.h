// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDShield(float Shield, float MaxShield);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
    void SetHUDWeaponAmmo(int32 Ammo);
    void SetHUDCarriedAmmo(int32 Ammo);
    void SetHUDWeaponType(EWeaponType WeaponType);
    void SetHUDMatchCountdown(float CountdownTime);
    void SetHUDAnnouncementCountdown(float CountdownTime);
    void SetHUDGrenades(int32 Grenades);
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastShowDefeatedAnimation();
    virtual void Tick(float DeltaSeconds) override;
    virtual float GetServerTime();
    virtual void ReceivedPlayer() override;
    void HandleStateChange();
    void OnMatchStateSet(FName State);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void CheckPingSpeed();
    float SingleTripTime = 0.0f;
    FHighPingDelegate HighPingDelegate;
protected:
    virtual void OnPossess(APawn* aPawn) override;
    virtual void BeginPlay() override;
    void SetHUDTime();
    void SetCharacterInputEnabled(bool bIsEnabled);
#pragma region GameTime
    
    UFUNCTION(Server, Reliable)
    void ServerRequestServerTime(float TimeOfClientRequest);

    UFUNCTION(Client, Reliable)
    void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

    float ClientServerDelta = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Time")
    float TimeSyncFrequency = 5.0f;

    float TimeSyncRunningTime = 0.0f;

    void CheckTimeSync(float DeltaSeconds);

#pragma endregion

    void PollInit();
    UFUNCTION(Server, Reliable)
    void ServerCheckMatchState();

    UFUNCTION(Client, Reliable)
    void ClientJoinMidGame(FName State, float Warmup, float Match, float Cooldown, float StartingTime);

    void HighPingWarning();
    void StopHighPingWarning();

    virtual void SetupInputComponent() override;
    void ShowReturnToMainMenu();
private:
    UPROPERTY(VisibleAnywhere)
    class ABlasterHUD* BlasterHUD;
    
    UPROPERTY(VisibleAnywhere)
    class ABlasterGameMode* BlasterGameMode;

    UPROPERTY(VisibleAnywhere)
    class UCharacterOverlay* CharacterOverlay;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UUserWidget> ReturnToMainMenuWidgetClass;

    UPROPERTY()
    class UReturnToMainMenu* ReturnToMainMenuWidget;

    bool bReturnToMainMenuOpen = false;
    
    bool IsHUDValid();

    float MatchTime = 0.0f;
    float WarmupTime = 0.0f;
    float CooldownTime = 0.0f;
    float LevelStartingTime = 0.0f;
    uint32 CountdownInt = 0;
    float WarningTimeThreshold = 30.0f; 

    UPROPERTY(ReplicatedUsing = "OnRep_MatchState")
    FName MatchState;

    UFUNCTION()
    void OnRep_MatchState();
    void HandleMatchHasStarted();
    void HandleCooldown();

    float HUDHealth;
    float HUDMaxHealth;
    bool bInitialiseHealth = false;
    
    float HUDShield;
    float HUDMaxShield;
    bool bInitialiseShield = false;
    
    float HUDScore;
    bool bInitialiseScore = false;
    
    int32 HUDDefeats;
    bool bInitialiseDefeats = false;
    
    int32 HUDGrenades;
    bool bInitialiseGrenades = false;

    int32 HUDWeaponAmmo;
    bool bInitialiseWeaponAmmo = false;

    int32 HUDCarriedAmmo;
    bool bInitialiseCarriedAmmo = false;

    EWeaponType HUDWeaponType;
    bool bInitialiseWeaponType = false;

    UPROPERTY(EditAnywhere)
    float HighPingDuration = 5.0f;
    UPROPERTY(EditAnywhere)
    float CheckPingFrequency = 20.0f;
    UPROPERTY(EditAnywhere)
    float HighPingThreshold = 50.0f;

    UFUNCTION(Server, Reliable)
    void ServerReportPingStatus(bool bHighPing);
};
