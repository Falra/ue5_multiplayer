// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DisplayText;

    void SetDisplayText(const FString TextToDisplay) const;
    void ShowPlayerNetRole(const APawn* InPawn) const;

protected:
    virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
