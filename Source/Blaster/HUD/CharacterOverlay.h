// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
    GENERATED_BODY()

public:

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* HealthText;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DefeatsAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeaponAmmoAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CarriedAmmoAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GrenadesAmount;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeaponType;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MatchCountdownText;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowDefeatedAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* BlinkCountdown;
};
