// Blaster Game. All Rights Reserved


#include "LagCompensationComponent.h"

#include "Blaster/Blaster.h"
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

    SaveFramePackageOnTick();
}

void ULagCompensationComponent::SaveFramePackageOnTick()
{
    if (!Character || !Character->HasAuthority()) return;

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
    Package.Character = Character;
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
    const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
    return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime) const
{
    const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
    return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters,
    const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime) const
{
    TArray<FFramePackage> FramesToCheck;
    for (const auto HitCharacter : HitCharacters)
    {
        FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
    }

    return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
    if (!HitCharacter || !HitCharacter->GetLagCompensationComponent()) return FFramePackage{};

    const auto LagComponent = HitCharacter->GetLagCompensationComponent();
    const auto& History = LagComponent->FrameHistory;
    if (!History.GetHead() || !History.GetTail()) return FFramePackage{};

    FFramePackage FrameToCheck;
    const float OldestHistoryTime = History.GetTail()->GetValue().Time;
    const float NewestHistoryTime = History.GetHead()->GetValue().Time;

    if (OldestHistoryTime > HitTime) return FFramePackage{}; // history is to old

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
    return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
    if (!Character || !HitCharacter || !DamageCauser) return;

    if (const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime); !Confirm.bHitConfirmed)
        return;

    UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), Character->Controller, DamageCauser, UDamageType::StaticClass());
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
    if (!Character || !HitCharacter || !Character->GetEquippedWeapon()) return;

    if (const FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime); !Confirm.bHitConfirmed)
        return;

    UGameplayStatics::ApplyDamage(HitCharacter, Character->GetEquippedWeapon()->GetDamage(), Character->Controller,
        Character->GetEquippedWeapon(), UDamageType::StaticClass());
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters,
    const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
    if (!Character || !DamageCauser) return;

    const FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

    for (auto& HitCharacter : HitCharacters)
    {
        if (!HitCharacter) continue;
        float TotalDamage = 0.0f;
        if (Confirm.HeadShots.Contains(HitCharacter))
        {
            TotalDamage += Confirm.HeadShots[HitCharacter] * DamageCauser->GetDamage();
        }
        if (Confirm.BodyShots.Contains(HitCharacter))
        {
            TotalDamage += Confirm.BodyShots[HitCharacter] * DamageCauser->GetDamage();
        }
        if (TotalDamage == 0.0f) continue;

        UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, Character->Controller, DamageCauser, UDamageType::StaticClass());
    }
    
}


FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame,
    float HitTime)
{
    const float Distance = YoungerFrame.Time - OlderFrame.Time;
    const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.0f, 1.0f);

    FFramePackage InterpFramePackage;
    InterpFramePackage.Time = HitTime;
    InterpFramePackage.Character = YoungerFrame.Character;

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
    if (!HitCharacter || !GetWorld()) return FServerSideRewindResult();

    FFramePackage CurrentFramePackage;
    CacheBoxPosition(HitCharacter, CurrentFramePackage);
    MoveBoxes(HitCharacter, Package);

    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Enable collision for the head first
    const auto HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    FHitResult ConfirmHitResult;
    const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
    GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
    if (ConfirmHitResult.bBlockingHit)
    {
        DrawDebugHitBox(ConfirmHitResult.Component, FColor::Red);
        
        ResetHitBoxes(HitCharacter, CurrentFramePackage);
        HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        return FServerSideRewindResult{true, true};
    }

    // Check for rest of the boxes
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HitBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    }
    GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);

    ResetHitBoxes(HitCharacter, CurrentFramePackage);
    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (ConfirmHitResult.bBlockingHit)
    {
        DrawDebugHitBox(ConfirmHitResult.Component, FColor::Blue);
        
        return FServerSideRewindResult{true, false};
    }

    return FServerSideRewindResult{false, false};
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime) const
{
    if (!HitCharacter || !GetWorld()) return FServerSideRewindResult();

    FFramePackage CurrentFramePackage;
    CacheBoxPosition(HitCharacter, CurrentFramePackage);
    MoveBoxes(HitCharacter, Package);

    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FPredictProjectilePathParams PathParams;
    PathParams.bTraceWithChannel = true;
    PathParams.bTraceWithCollision = true;
    PathParams.MaxSimTime = MaxRecordTime;
    PathParams.StartLocation = TraceStart;
    PathParams.LaunchVelocity = InitialVelocity;
    PathParams.SimFrequency = 15.0f;
    PathParams.ProjectileRadius = 5.0f;
    PathParams.TraceChannel = ECC_HitBox;
    PathParams.ActorsToIgnore.Add(GetOwner());
    PathParams.DrawDebugTime = 5.0f;
    PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
    
    FPredictProjectilePathResult PathResult;
    
    // Enable collision for the head first
    const auto HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    
    UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
    if (PathResult.HitResult.bBlockingHit)
    {
        DrawDebugHitBox(PathResult.HitResult.Component, FColor::Red);
        
        ResetHitBoxes(HitCharacter, CurrentFramePackage);
        HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        return FServerSideRewindResult{true, true};
    }

    // Check for rest of the boxes
    for (auto& [HitBoxName, HitBox] : HitCharacter->HitCollisionBoxes)
    {
        if (!HitBox) continue;
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HitBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    }
    
    UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
    
    ResetHitBoxes(HitCharacter, CurrentFramePackage);
    HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (PathResult.HitResult.bBlockingHit)
    {
        DrawDebugHitBox(PathResult.HitResult.Component, FColor::Blue);
        
        return FServerSideRewindResult{true, false};
    }
    
    return FServerSideRewindResult{false, false};
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& Packages,
    const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const
{
    if (!GetWorld()) return FShotgunServerSideRewindResult{};
    
    FShotgunServerSideRewindResult ShotgunResult;
    TArray<FFramePackage> CurrentFrames;
    for (auto& FramePackage : Packages)
    {
        if (!FramePackage.Character) continue;
        
        FFramePackage CurrentFramePackage;
        CurrentFramePackage.Character = FramePackage.Character;
        CacheBoxPosition(FramePackage.Character, CurrentFramePackage);
        MoveBoxes(FramePackage.Character, FramePackage);
        FramePackage.Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        CurrentFrames.Add(CurrentFramePackage);
    }

    for (auto& FramePackage : Packages)
    {
        if (!FramePackage.Character) continue;
        
        // Enable collision for the head first
        const auto HeadBox = FramePackage.Character->HitCollisionBoxes[FName("head")];
        HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    }

    // check for headshots
    for (auto& HitLocation : HitLocations)
    {
        FHitResult ConfirmHitResult;
        const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
        GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);

        if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor()))
        {
            DrawDebugHitBox(ConfirmHitResult.Component, FColor::Red);
            
            auto& HitMap = ShotgunResult.HeadShots;
            HitMap.Contains(BlasterCharacter) ? HitMap[BlasterCharacter]++ : HitMap.Emplace(BlasterCharacter, 1);
        }
    }

    // Check for rest of the boxes
    for (auto& FramePackage : Packages)
    {
        for (auto& [HitBoxName, HitBox] : FramePackage.Character->HitCollisionBoxes)
        {
            if (!HitBox) continue;
            HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            HitBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
        }
        const auto HeadBox = FramePackage.Character->HitCollisionBoxes[FName("head")];
        HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // check for body shots
    for (auto& HitLocation : HitLocations)
    {
        FHitResult ConfirmHitResult;
        const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
        GetWorld()->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);

        if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor()))
        {
            DrawDebugHitBox(ConfirmHitResult.Component, FColor::Blue);
            
            auto& HitMap = ShotgunResult.BodyShots;
            HitMap.Contains(BlasterCharacter) ? HitMap[BlasterCharacter]++ : HitMap.Emplace(BlasterCharacter, 1);
        }
    }

    // Reset positions
    for (auto& CurrentFramePackage : CurrentFrames)
    {
        ResetHitBoxes(CurrentFramePackage.Character, CurrentFramePackage);
        CurrentFramePackage.Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    return ShotgunResult;
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

void ULagCompensationComponent::DrawDebugHitBox(const TWeakObjectPtr<UPrimitiveComponent> HitComponent, const FColor& DrawColor) const
{
    if (!bDrawDebug) return;
    
    if (!HitComponent.IsValid()) return;

    if (const UBoxComponent* Box = Cast<UBoxComponent>(HitComponent))
    {
        DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), DrawColor,
            false, 8.f);
    }
}
