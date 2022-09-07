// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetElimAnnouncement(FString AttackerName, FString VictimName);
    
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* AnnouncementBox;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* AnnouncementText;
};
