// Blaster Game. All Rights Reserved


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!GEngine) return;
    
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    const FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

    if (HUDPackage.CrosshairsCenter)
    {
        DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter);
    }

    if (HUDPackage.CrosshairsLeft)
    {
        DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter);
    }

    if (HUDPackage.CrosshairsRight)
    {
        DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter);
    }

    if (HUDPackage.CrosshairsTop)
    {
        DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter);
    }

    if (HUDPackage.CrosshairsBottom)
    {
        DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter);
    }
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D DrawPoint(ViewportCenter.X - (TextureWidth / 2.0f), ViewportCenter.Y - (TextureHeight / 2.0f));
    DrawTexture(Texture, DrawPoint.X, DrawPoint.Y, TextureWidth, TextureHeight, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::White);
}
