// Blaster Game. All Rights Reserved


#include "Pickup.h"

#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    OverlapSphere = CreateDefaultSubobject<USphereComponent>("OverlapSphere");
    OverlapSphere->SetupAttachment(RootComponent);
    OverlapSphere->SetSphereRadius(150.0f);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    OverlapSphere->AddLocalOffset(FVector(0.0f, 0.0f, 85.0f));

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetupAttachment(OverlapSphere);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMesh->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));
    PickupMesh->SetRenderCustomDepth(true);
    PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
}

void APickup::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
    }
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void APickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (PickupMesh && BaseTurnRate != 0.0f)
    {
        PickupMesh->AddWorldRotation(FRotator(0.0f, BaseTurnRate * DeltaTime, 0.0f));
    }
}

void APickup::Destroyed()
{
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
        
    Super::Destroyed();
}
