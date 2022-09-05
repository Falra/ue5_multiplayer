// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    void MenuSetup();
    void MenuTearDown();
protected:
    virtual bool Initialize() override;
private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ReturnButton;

    UFUNCTION()
    void ReturnButtonClicked();
};
