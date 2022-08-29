// Blaster Game. All Rights Reserved


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/BoxComponent.h"

ULagCompensationComponent::ULagCompensationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
    Character = !Character ? Cast<ABlasterCharacter>(GetOwner()) : Character;
    if (!Character) return;
    Package.Time = GetWorld()->GetTimeSeconds();
    for (auto& [BoxName, BoxComponent] : Character->HitCollisionBoxes)
    {
        FBoxInformation BoxInformation;
        BoxInformation.Location = BoxComponent->GetComponentLocation();
        BoxInformation.Rotation = BoxComponent->GetComponentRotation();
        BoxInformation.BoxExtent = BoxComponent->GetScaledBoxExtent();
        Package.HitBoxInfo.Add(BoxName, BoxInformation);
    }
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color) const
{
    for (auto& [BoxName, BoxComponent]: Package.HitBoxInfo)
    {
        DrawDebugBox(GetWorld(), BoxComponent.Location, BoxComponent.BoxExtent, FQuat(BoxComponent.Rotation), Color, false, 4.0f);
    }
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FrameHistory.Num() > 1)
    {
        float HistoryLenght = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
        while (HistoryLenght > MaxRecordTime)
        {
            FrameHistory.RemoveNode(FrameHistory.GetTail());
            HistoryLenght = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
        }
    }
    
    FFramePackage ThisFrame;
    SaveFramePackage(ThisFrame);
    FrameHistory.AddHead(ThisFrame);
    
    ShowFramePackage(ThisFrame, FColor::Purple);
}