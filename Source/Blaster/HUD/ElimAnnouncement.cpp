// Blaster Game. All Rights Reserved


#include "ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncement(FString AttackerName, FString VictimName)
{
    if (!AnnouncementText) return;

    FString ElimAnnouncementText = FString::Printf(TEXT("%s eliminated %s"), *AttackerName, *VictimName);
    AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
}
