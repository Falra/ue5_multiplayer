// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
    GENERATED_BODY()
public:
    
    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsCenter;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsLeft;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsRight;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsTop;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsBottom;

    float CrosshairSpread;

    FLinearColor CrosshairsColor; 
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    void AddCharacterOverlay();
    void AddAnnouncementWidget();
    void AddElimAnnouncementWidget(FString AttackerName, FString VictimName);
    
    UPROPERTY(EditAnywhere, Category = "Player stats")
    TSubclassOf<UUserWidget> CharacterOverlayClass;
    
    UPROPERTY(VisibleAnywhere, Category = "Player stats")
    class UCharacterOverlay* CharacterOverlay;

    UPROPERTY(EditAnywhere, Category = "Announcement")
    TSubclassOf<UUserWidget> AnnouncementWidgetClass;

    UPROPERTY(VisibleAnywhere, Category = "Announcement")
    class UAnnouncement* AnnouncementWidget;

    UPROPERTY(EditAnywhere, Category = "Elim Announcement")
    TSubclassOf<UUserWidget> ElimAnnouncementWidgetClass;
    
protected:
    virtual void BeginPlay() override;
    
private:
    UPROPERTY()
    class APlayerController* OwningPlayer;

    FHUDPackage HUDPackage;

    void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

    UPROPERTY(EditAnywhere)
    float CrosshairSpreadMax = 16.0f;
public:
    FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
