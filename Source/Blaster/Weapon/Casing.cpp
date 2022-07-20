// Blaster Game. All Rights Reserved


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
    PrimaryActorTick.bCanEverTick = false;

    CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
    SetRootComponent(CasingMesh);
    CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    CasingMesh->SetSimulatePhysics(true);
    CasingMesh->SetNotifyRigidBodyCollision(true);
    CasingMesh->SetEnableGravity(true);
}

void ACasing::BeginPlay()
{
    Super::BeginPlay();
    CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
    CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (ShellSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
    }

    Destroy();
}
