// Blaster Game. All Rights Reserved


#include "Pickup.h"

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

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetupAttachment(OverlapSphere);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
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
}

void APickup::Destroyed()
{
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
        
    Super::Destroyed();
}
