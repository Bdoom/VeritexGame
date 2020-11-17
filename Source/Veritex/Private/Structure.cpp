// Daniel Gleason (C) 2017

#include "Structure.h"
#include "Veritex.h"
#include "Kismet/KismetMathLibrary.h"
#include "VeritexGameMode.h"
#include "DestructibleMesh.h"
#include "VeritexFunctionLibrary.h"
#include "VeritexDestructibleActor.h"

// Debug Helpers
#include "DrawDebugHelpers.h"

// Sets default values
AStructure::AStructure()
{
	PrimaryActorTick.bCanEverTick = false;
	Structure = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	RootComponent = Root;
	Structure->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	bCanBeDamaged = true;
	Structure->SetMobility(EComponentMobility::Stationary);

	//NetUpdateFrequency = 25;
	//NetDormancy = ENetDormancy::DORM_Initial;

	static ConstructorHelpers::FObjectFinder<USoundCue> DestroyedSoundCue(TEXT("SoundCue'/Game/Sounds/ProSoundCollection_v1_3_MONO/Impacts_Smashable/Cues/RockSmashCue.RockSmashCue'"));
	if (DestroyedSoundCue.Object)
	{
		DestroyedSound = DestroyedSoundCue.Object;
	}

	//NetDormancy = ENetDormancy::DORM_DormantAll;
	//NetUpdateFrequency = 0.01;
	//NetPriority = 0.5;

	//NetDormancy = DORM_Initial;
	//NetUpdateFrequency = 66.0f;
	//MinNetUpdateFrequency = 2.f;
	//NetCullDistanceSquared = 3000000;//500000.0;

	bReplicates = true;
	bReplicateMovement = true;

	bIsHibernating = false;
}

bool AStructure::IsWaterCraft_Implementation() const
{
	return false;
}

bool AStructure::IsPossessable_Implementation() const
{
	if (StructureType == EStructureType::Foundation || StructureType == EStructureType::Ceiling)
	{
		return true;
	}
	else
	{
		return false;
	}
}

TMap<FString, FColor> AStructure::GetColorMap()
{
	TMap<FString, FColor> ColorMap;

	for (int i = 0; i < GColorList.GetColorsNum(); i++)
	{
		FString ColorName = GColorList.GetColorNameByIndex(i);
		FColor Color = GColorList.GetFColorByName(*ColorName);

		ColorMap.Add(ColorName, Color);
	}

	return ColorMap;
}

UMaterialInstanceDynamic* AStructure::CreateDynamicInstance()
{
	if (!DynamicMaterialInstance)
	{
		UMaterialInterface* Mat = Structure->GetMaterial(0);
		if (Mat)
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Mat, this);
			Structure->SetMaterial(0, DynamicMaterialInstance);
		}
	}
	return DynamicMaterialInstance;
}

void AStructure::OnRep_PaintStructure()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), FLinearColor::FromSRGBColor(StructureColor));
	}
	else
	{
		CreateDynamicInstance()->SetVectorParameterValue(FName("Color"), FLinearColor::FromSRGBColor(StructureColor));
	}
}

void AStructure::PostNetInit()
{
	Super::PostNetInit();
	//CreateDynamicInstance()->SetVectorParameterValue(FName("Color"), FLinearColor::FromSRGBColor(StructureColor));
}

float AStructure::BoundingBoxTraces()
{
	UWorld* World = GetWorld();

	FVector Origin;
	FVector Extents;
	float SphereRadius;
	UKismetSystemLibrary::GetComponentBounds(Structure, Origin, Extents, SphereRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult LineTraceOne;
	FVector LineTraceOneStart = Origin + Extents + FVector(Extents.X * -2, 0, 500);
	FVector LineTraceOneEnd = Origin + Extents + FVector(Extents.X * -2, 0, -1000.f);
	World->LineTraceSingleByChannel(LineTraceOne, LineTraceOneStart, LineTraceOneEnd, ECC_Visibility, QueryParams);
	//DrawDebugLine(World, LineTraceOneStart, LineTraceOneEnd, FColor::Red, true);

	FHitResult LineTraceTwo;
	FVector LineTraceTwoStart = Origin + Extents + FVector(0, 0, 500);
	FVector LineTraceTwoEnd = Origin + Extents + FVector(0, 0, -1000);
	World->LineTraceSingleByChannel(LineTraceTwo, LineTraceTwoStart, LineTraceTwoEnd, ECC_Visibility, QueryParams);
	//DrawDebugLine(World, LineTraceTwoStart, LineTraceTwoEnd, FColor::Red, true);

	FHitResult LineTraceThree;
	FVector LineTraceThreeStart = Origin + Extents + FVector(0, Extents.Y * -2, 500);
	FVector LineTraceThreeEnd = Origin + Extents + FVector(0, Extents.Y * -2, -1000);
	World->LineTraceSingleByChannel(LineTraceThree, LineTraceThreeStart, LineTraceThreeEnd, ECC_Visibility, QueryParams);
	//DrawDebugLine(World, LineTraceThreeStart, LineTraceThreeEnd, FColor::Red, true);

	FHitResult LineTraceFour;
	FVector LineTraceFourStart = Origin + Extents + FVector(Extents.X * -2, Extents.Y * -2, 500);
	FVector LineTraceFourEnd = Origin + Extents + FVector(Extents.X * -2, Extents.Y * -2, -1000);
	World->LineTraceSingleByChannel(LineTraceFour, LineTraceFourStart, LineTraceFourEnd, ECC_Visibility, QueryParams);
	//DrawDebugLine(World, LineTraceFourStart, LineTraceFourEnd, FColor::Red, true);

	TArray<FHitResult> Hits;
	Hits.Add(LineTraceOne);
	Hits.Add(LineTraceTwo);
	Hits.Add(LineTraceThree);
	Hits.Add(LineTraceFour);

	float HighestZ = -1000000;

	for (FHitResult Hit : Hits)
	{
		if (Hit.Location.Z > HighestZ)
		{
			HighestZ = Hit.Location.Z;
		}
	}

	return HighestZ;
}

void AStructure::OnHibernationModeChanged_Implementation()
{
	if (IsHibernationModeEnabled())
	{
		NetUpdateFrequency = 0.01;
		SetReplicateMovement(false);
		SetReplicates(false);
		SetNetDormancy(DORM_DormantAll);
	}
	else
	{
		NetUpdateFrequency = GetDefault<AActor>(GetClass())->NetUpdateFrequency;
		SetReplicateMovement(true);
		SetReplicates(true);
		SetNetDormancy(DORM_Awake);
	}
}

void AStructure::SetHibernationMode(bool bHibernating)
{
	if (bIsHibernating != bHibernating)
	{
		bIsHibernating = bHibernating;
		OnHibernationModeChanged();
	}
}

FHitResult& AStructure::CalculateBoundsTrace()
{
	FVector Origin;
	FVector BoxExtent;
	float SphereRadius;

	UKismetSystemLibrary::GetComponentBounds(Structure, Origin, BoxExtent, SphereRadius);

	FVector Top = Origin + FVector(0, -BoxExtent.Y, BoxExtent.Z + 500);
	FVector Bottom = Origin - FVector(0, BoxExtent.Y, BoxExtent.Z);

	//DrawDebugLine(GetWorld(), Top, Bottom, FColor::Red, true);
	GetWorld()->LineTraceSingleByChannel(CalcBoundsOutHitOne, Top, Bottom, ECollisionChannel::ECC_Visibility);
	// Do the four traces, from all sides, then check for which one has the highest Z value and then return that value.

// 	FVector SecondTraceTop = Origin + BoxExtent + FVector(-800, 0, 500);
// 	FVector SecondTraceBottom = Origin + BoxExtent + FVector(-800, 0, 0);
// 	DrawDebugLine(GetWorld(), SecondTraceTop, SecondTraceBottom, FColor::Red, true);
// 	GetWorld()->LineTraceSingleByChannel(CalcBoundsOutHitTwo, SecondTraceTop, SecondTraceBottom, ECollisionChannel::ECC_Visibility);

	return CalcBoundsOutHitOne;
}

// Called when the game starts or when spawned
void AStructure::BeginPlay()
{
	Super::BeginPlay();

	// Set Structure Quality and Type Enums
	if (HasAuthority())
	{
		AVeritexGameMode* VGM = GetWorld()->GetAuthGameMode<AVeritexGameMode>();
		if (VGM)
		{
			VGM->Structures.AddUnique(this);
		}
	}

// 	UMaterialInterface* Mat = Structure->GetMaterial(0);
// 	if (Mat)
// 	{
// 		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Mat, this);
// 		Structure->SetMaterial(0, DynamicMaterialInstance);
// 	}

	switch (StructureType)
	{
	case EStructureType::Foundation:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 5000;
				StructureMaxHealth = 5000;
			}
			StructureName = "Wood Foundation";
		}
		if (StructureQuality == EStructureQuality::Stone)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 10000;
				StructureMaxHealth = 10000;
			}
			StructureName = "Stone Foundation";
		}
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 35000;
				StructureMaxHealth = 35000;
			}
			StructureName = "Metal Foundation";
		}
		if (StructureQuality == EStructureQuality::Thatch)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2500;
				StructureMaxHealth = 2500;
			}
			StructureName = "Thatch Foundation";
		}
		break;
	case EStructureType::Wall:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 1500;
				StructureMaxHealth = 1500;
			}
			StructureName = "Wood Wall";
		}
		if (StructureQuality == EStructureQuality::Stone)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 7500;
				StructureMaxHealth = 7500;
			}
			StructureName = "Stone Wall";
		}
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 25000;
				StructureMaxHealth = 25000;
			}
			StructureName = "Metal Wall";
		}
		if (StructureQuality == EStructureQuality::Thatch)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2500;
				StructureMaxHealth = 2500;
			}
			StructureName = "Thatch Wall";
		}
		break;
	case EStructureType::DoorFrame:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 1500;
				StructureMaxHealth = 1500;
			}
			StructureName = "Wood Door frame";
		}
		if (StructureQuality == EStructureQuality::Stone)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 7500;
				StructureMaxHealth = 7500;
			}
			StructureName = "Stone Door frame";
		}
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 15000;
				StructureMaxHealth = 15000;
			}
			StructureName = "Metal Door frame";
		}
		if (StructureQuality == EStructureQuality::Thatch)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2250;
				StructureMaxHealth = 2250;
			}
			StructureName = "Thatch Door frame";
		}
		break;
	case EStructureType::Door:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 500;
				StructureMaxHealth = 500;
			}
			StructureName = "Wood Door";
		}
		if (StructureQuality == EStructureQuality::Thatch)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 250;
				StructureMaxHealth = 250;
			}
			StructureName = "Thatch Door";
		}
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 10000;
				StructureMaxHealth = 10000;
			}
			StructureName = "Metal Door";
		}
		break;
	case EStructureType::Ceiling:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2000;
				StructureMaxHealth = 2000;
			}
			StructureName = "Wood Ceiling";
		}
		if (StructureQuality == EStructureQuality::Stone)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 5000;
				StructureMaxHealth = 5000;
			}
			StructureName = "Stone Ceiling";
		}
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 10000;
				StructureMaxHealth = 10000;
			}
			StructureName = "Metal Ceiling";
		}
		if (StructureQuality == EStructureQuality::Thatch)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 1500;
				StructureMaxHealth = 1500;
			}
			StructureName = "Thatch Ceiling";
		}
		break;

	case EStructureType::Storage_Small:

		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2500;
				StructureMaxHealth = 2500;
			}
		}
		StructureName = "Small Storage Box";
		break;

	case EStructureType::StairCase:
		if (StructureQuality == EStructureQuality::Wood)
		{

			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 3000;
				StructureMaxHealth = 3000;
			}
			StructureName = "Wood Stairs";
		}
		break;

	case EStructureType::Bed:
		if (StructureQuality == EStructureQuality::Wood || StructureQuality == EStructureQuality::Thatch)
		{

			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 500;
				StructureMaxHealth = 500;
			}
			StructureName = "Bed";
		}
		break;

	case EStructureType::Turret:
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 15000;
				StructureMaxHealth = 15000;
			}
			StructureName = "Autonomous Turret";
		}
		break;

	case EStructureType::CampFire:
		if (StructureQuality == EStructureQuality::Wood)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 150;
				StructureMaxHealth = 150;
			}
			StructureName = "Camp Fire";
		}
		break;

	case EStructureType::Furnace:
		if (StructureQuality == EStructureQuality::Metal)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 2500;
				StructureMaxHealth = 2500;
			}
			StructureName = "Furnace";
		}
		break;
	case EStructureType::PartyStone:
		if (StructureQuality == EStructureQuality::Stone)
		{
			if (StructureHealth == 0 || StructureMaxHealth == 0)
			{
				StructureHealth = 50;
				StructureMaxHealth = 50;
			}
			StructureName = "Party Stone";
		}

		break;
	case EStructureType::WoodPile:
		if (StructureHealth == 0 || StructureMaxHealth == 0)
		{
			StructureHealth = 500;
			StructureMaxHealth = 500;
		}

		StructureName = "Wood Pile";
		break;

	case EStructureType::StonePile:
		if (StructureHealth == 0 || StructureMaxHealth == 0)
		{
			StructureHealth = 500;
			StructureMaxHealth = 500;
		}

		StructureName = "Stone Pile";
		break;

	case EStructureType::FoodPile:
		if (StructureHealth == 0 || StructureMaxHealth == 0)
		{
			StructureHealth = 500;
			StructureMaxHealth = 500;
		}

		StructureName = "Food Pile";
		break;
	}
}

float AStructure::GetDistanceBetweenPoints(FVector A, FVector B)
{
	return FVector::Dist(A, B);
}

float AStructure::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	StructureHealth -= Damage;

	if (StructureHealth <= 0)
	{
		StructureHealth = 0;

		if (DM_Mesh)
		{
			if (DM_Mesh->ApexDestructibleAsset)
			{
				AVeritexDestructibleActor* VDA = GetWorld()->SpawnActor<AVeritexDestructibleActor>(AVeritexDestructibleActor::StaticClass(), GetActorTransform());
				Multicast_SetDestructibleMeshOnDMActor(VDA);
				Multicast_SpawnSoundOnDestruction();
			}
		}

		if (IsValid(this))
		{
			Destroy();
		}

	}

	return Damage;
}

void AStructure::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		AVeritexGameMode* VGM = GetWorld()->GetAuthGameMode<AVeritexGameMode>();
		if (VGM)
		{
			VGM->Structures.RemoveSingle(this);
		}

		ANPC* NPC_Raft = Cast<ANPC>(GetAttachParentActor());
		if (NPC_Raft) // On Destroy, remove this structure from the raft list so it is not saved. 
		{
			NPC_Raft->StructuresAttached.Remove(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AStructure::Multicast_SpawnSoundOnDestruction_Implementation()
{
	if (DestroyedSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DestroyedSound, GetActorLocation());
	}
}

bool AStructure::Multicast_SpawnSoundOnDestruction_Validate()
{
	return true;
}

void AStructure::Multicast_SetDestructibleMeshOnDMActor_Implementation(AVeritexDestructibleActor* VDA)
{
	TWeakObjectPtr<AVeritexDestructibleActor> WeakVDA = VDA;
	if (WeakVDA.IsValid())
	{
		if (DM_Mesh)
		{
			if (DM_Mesh->ApexDestructibleAsset)
			{
				VDA->DM_Component->SetDestructibleMesh(DM_Mesh);
			}
		}
	}
}

bool AStructure::Multicast_SetDestructibleMeshOnDMActor_Validate(AVeritexDestructibleActor* VDA)
{
	// Check for cheats
	return true;
}

void AStructure::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStructure, StructureMaxHealth);
	DOREPLIFETIME(AStructure, StructureHealth);
	DOREPLIFETIME(AStructure, OwnedBy);
	DOREPLIFETIME(AStructure, StructureName);
	DOREPLIFETIME(AStructure, StructureColor);
	//DOREPLIFETIME(AStructure, DB_Key_Unique);
	//DOREPLIFETIME(AStructure, NeededTribeRank);
	if (StructureType == EStructureType::Door)
	{
		DOREPLIFETIME(AStructure, bIsDoorOpen);
	}

	DOREPLIFETIME(AStructure, StructureOwners);

}