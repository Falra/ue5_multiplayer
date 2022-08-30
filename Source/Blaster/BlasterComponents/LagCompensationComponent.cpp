// Blaster Game. All Rights Reserved


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
    Super::BeginPlay();
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
    for (auto& [BoxName, BoxComponent] : Package.HitBoxInfo)
    {
        DrawDebugBox(GetWorld(), BoxComponent.Location, BoxComponent.BoxExtent, FQuat(BoxComponent.Rotation), Color, false, 4.0f);
    }
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize& HitLocation, float HitTime) const
{
    if (!HitCharacter || !HitCharacter->GetLagCompensationComponent()) return FServerSideRewindResult{};

    const auto LagComponent = HitCharacter->GetLagCompensationComponent();
    const auto& History = LagComponent->FrameHistory;
    if (!History.GetHead() || !History.GetTail()) return FServerSideRewindResult{};

    FFramePackage FrameToCheck;
    const float OldestHistoryTime = History.GetTail()->GetValue().Time;
    const float NewestHistoryTime = History.GetHead()->GetValue().Time;

    if (OldestHistoryTime > HitTime) return FServerSideRewindResult{}; // history is to old

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
        FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
    }

    return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
    if (!HitCharacter || !DamageCauser) return;

    if (const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime); !Confirm.bHitConfirmed)
        return;

    UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), Character->Controller, DamageCauser, UDamageType::StaticClass());
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame,
    float HitTime) const
{
    const float Distance = YoungerFrame.Time - OlderFrame.Time;
    const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.0f, 1.0f);

    FFramePackage InterpFramePackage;
    InterpFramePackage.Time = HitTime;

    for (const auto& [BoxInfoName, YoungerBoxInfo] : YoungerFrame.HitBoxInfo)
    {
        const auto& OlderBoxInfo = OlderFrame.HitBoxInfo[BoxInfoName];

        FBoxInformation InterpBoxInfo;

        InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.0f, InterpFraction);
        InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.0f, InterpFraction);
        InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;

        InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
    }
    return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const
{
    if (!HitCharacter || GetWorld()) return FServerSideRewindResult();

    FFramePackage CurrentFramePackage;
    CacheBoxPosition(HitCharacter, CurrentFramePackage);
    MoveBoxes(HitCharacter, Package);

    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Enable collision for the head first
    const auto HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    FHitResult ConfirmHitResult;
    const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
    GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);
    if (ConfirmHitResult.bBlockingHit)
    {
        ResetHitBoxes(HitCharacter, CurrentFramePackage);
        HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        return FServerSideRewindResult{true, true};
    }

    // Check for rest of the boxes
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    }
    GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

    ResetHitBoxes(HitCharacter, CurrentFramePackage);
    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (ConfirmHitResult.bBlockingHit)
    {
        return FServerSideRewindResult{true, false};
    }

    return FServerSideRewindResult{false, false};
}

void ULagCompensationComponent::CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
    if (!HitCharacter) return;
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        FBoxInformation BoxInfo;
        BoxInfo.Location = HitBox->GetComponentLocation();
        BoxInfo.Rotation = HitBox->GetComponentRotation();
        BoxInfo.BoxExtent = HitBox->GetScaledBoxExtent();
        OutFramePackage.HitBoxInfo.Add(HitBoxName, BoxInfo);
    }
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
    if (!HitCharacter) return;
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        const auto& [Location, Rotation, BoxExtent] = Package.HitBoxInfo[HitBoxName];
        HitBox->SetWorldLocation(Location);
        HitBox->SetWorldRotation(Rotation);
        HitBox->SetBoxExtent(BoxExtent);
    }
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
    if (!HitCharacter) return;
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        const auto& [Location, Rotation, BoxExtent] = Package.HitBoxInfo[HitBoxName];
        HitBox->SetWorldLocation(Location);
        HitBox->SetWorldRotation(Rotation);
        HitBox->SetBoxExtent(BoxExtent);
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}
