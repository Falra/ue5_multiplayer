// Blaster Game. All Rights Reserved


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!GEngine) return;
    
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    const FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
    const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
    
    DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, {0.0f, 0.0f});
    DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, {-SpreadScaled, 0.0f});
    DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, {SpreadScaled, 0.0f});
    DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, {0.0f, -SpreadScaled});
    DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, {0.0f, SpreadScaled});
    
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
    if (!Texture) return;
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D DrawPoint(ViewportCenter.X - (TextureWidth / 2.0f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.0f) + Spread.Y);
    DrawTexture(Texture, DrawPoint.X, DrawPoint.Y, TextureWidth, TextureHeight, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::White);
}
