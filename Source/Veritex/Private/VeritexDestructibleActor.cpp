// Daniel Gleason (C) 2017


#include "VeritexDestructibleActor.h"
#include "Veritex.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"


// Sets default values
AVeritexDestructibleActor::AVeritexDestructibleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	DM_Component = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DM_Component"));
	DM_Component->SetupAttachment(GetRootComponent());
	DM_Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//DM_Component->SetIsReplicated(true);


// 	static ConstructorHelpers::FObjectFinder<UDestructibleMesh> DMesh(TEXT("DestructibleMesh'/Game/StarterContent/Shapes/Shape_Cube_DM.Shape_Cube_DM'"));
// 	if (DMesh.Object)
// 	{
// 		DM_Component->SetDestructibleMesh(DMesh.Object);
// 	}

	
	bReplicates = true;
}

// Called when the game starts or when spawned
void AVeritexDestructibleActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SetLifeSpan(2.f);
		Multicast_ApplyDestructibleDamage();
	}

	if (!HasAuthority())
	{
		if (SoundToPlay)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
		}
	}

}

void AVeritexDestructibleActor::Multicast_ApplyDestructibleDamage_Implementation()
{
	DM_Component->ApplyDamage(10, DM_Component->GetComponentLocation(), DM_Component->GetComponentLocation(), 200.f);
}

bool AVeritexDestructibleActor::Multicast_ApplyDestructibleDamage_Validate()
{
	return true;
}


