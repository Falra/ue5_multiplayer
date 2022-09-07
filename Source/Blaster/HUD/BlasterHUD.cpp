// Blaster Game. All Rights Reserved


#include "BlasterHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!GEngine) return;
    
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    const FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
    const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
    
    DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, {0.0f, 0.0f}, HUDPackage.CrosshairsColor);
    DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, {-SpreadScaled, 0.0f}, HUDPackage.CrosshairsColor);
    DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, {SpreadScaled, 0.0f}, HUDPackage.CrosshairsColor);
    DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, {0.0f, -SpreadScaled}, HUDPackage.CrosshairsColor);
    DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, {0.0f, SpreadScaled}, HUDPackage.CrosshairsColor);
    
}

void ABlasterHUD::BeginPlay()
{
    Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
    const auto PlayerController = GetOwningPlayerController();
    if (!PlayerController || !CharacterOverlayClass) return;
    CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
    CharacterOverlay->AddToViewport();
}

void ABlasterHUD::AddAnnouncementWidget()
{
    const auto PlayerController = GetOwningPlayerController();
    if (!PlayerController || !AnnouncementWidgetClass || AnnouncementWidget) return;
    AnnouncementWidget = CreateWidget<UAnnouncement>(PlayerController, AnnouncementWidgetClass);
    AnnouncementWidget->AddToViewport();
}

void ABlasterHUD::AddElimAnnouncementWidget(FString AttackerName, FString VictimName)
{
    OwningPlayer = !OwningPlayer ? GetOwningPlayerController() : OwningPlayer;
    if (!OwningPlayer || !ElimAnnouncementWidgetClass) return;
    UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementWidgetClass);
    if (ElimAnnouncement)
    {
        ElimAnnouncement->SetElimAnnouncement(AttackerName, VictimName);
        ElimAnnouncement->AddToViewport();
    }
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor)
{
    if (!Texture) return;
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D DrawPoint(ViewportCenter.X - (TextureWidth / 2.0f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.0f) + Spread.Y);
    DrawTexture(Texture, DrawPoint.X, DrawPoint.Y, TextureWidth, TextureHeight, 0.0f, 0.0f, 1.0f, 1.0f, CrosshairsColor);
}
