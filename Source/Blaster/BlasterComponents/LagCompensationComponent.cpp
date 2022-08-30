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

void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize& HitLocation, float HitTime)
{
    if (!HitCharacter || !HitCharacter->GetLagCompensationComponent()) return;

    const auto LagComponent = HitCharacter->GetLagCompensationComponent();
    const auto& History = LagComponent->FrameHistory;
    if (!History.GetHead() || !History.GetTail()) return;

    FFramePackage FrameToCheck;
    const float OldestHistoryTime = History.GetTail()->GetValue().Time;
    const float NewestHistoryTime = History.GetHead()->GetValue().Time;
    
    if (OldestHistoryTime > HitTime) return; // history is to old

    bool bShouldInterpolate = true;
    if (OldestHistoryTime == HitTime)
    {
        FrameToCheck = History.GetTail()->GetValue();
        bShouldInterpolate = false;
    }
    else if (NewestHistoryTime <= HitTime)
    {
        FrameToCheck = History.GetHead()->GetValue();
        bShouldInterpolate = false;
    }

    auto Younger = History.GetHead();
    auto Older = Younger;
    while (Older->GetValue().Time > HitTime)
    {
        if (!Older->GetNextNode()) break;
        Older = Older->GetNextNode();
        if ((Older->GetValue().Time > HitTime))
        {
            Younger = Older;
        }
    }

    if (Older->GetValue().Time == HitTime)
    {
        FrameToCheck = Older->GetValue();
        bShouldInterpolate = false;
    }

    if (bShouldInterpolate)
    {
        // TODO: Interpolate b/w older and younger frames
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
    
    // ShowFramePackage(ThisFrame, FColor::Purple);
}
