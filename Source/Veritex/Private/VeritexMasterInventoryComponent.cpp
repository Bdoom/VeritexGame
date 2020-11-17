// Daniel Gleason (C) 2017

#include "VeritexMasterInventoryComponent.h"
#include "Veritex.h"
#include "TimerManager.h"
#include "VeritexFunctionLibrary.h"


// Sets default values for this component's properties
UVeritexMasterInventoryComponent::UVeritexMasterInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ShouldCallOnInventoryReady = true;

	bReplicates = true;

	// ...
}


void UVeritexMasterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UVeritexMasterInventoryComponent, MaxWeight);
}

bool UVeritexMasterInventoryComponent::GetShouldCallOnInventoryReady() const
{
	return ShouldCallOnInventoryReady;
}

void UVeritexMasterInventoryComponent::SetShouldCallOnInventoryReady(bool NewValue)
{
	ShouldCallOnInventoryReady = NewValue;
}

UArrayProperty* UVeritexMasterInventoryComponent::GetInventoryArray()
{
	for (TFieldIterator<UProperty> PropIt(GetClass()); PropIt; ++PropIt)
	{
		UProperty* Property{ *PropIt };
		FString PropertyName{ GetNameSafe(Property) };
		if (PropertyName == "Inventory")
		{
			UArrayProperty* InventoryArray{ Cast<UArrayProperty>(Property) };
			if (InventoryArray)
			{
				return InventoryArray;
			}
		}
	}
	return nullptr;
}

TArray<FItemData> UVeritexMasterInventoryComponent::GetInventory()
{
	UArrayProperty* InventoryArray = GetInventoryArray();
	TArray<FItemData> Items;

	if (InventoryArray)
	{

		FScriptArrayHelper PropertyHelper{ InventoryArray, InventoryArray->ContainerPtrToValuePtr<void>(this) };
		UProperty* InnerProperty{ InventoryArray->Inner };

		UStructProperty* StructProperty{ Cast<UStructProperty>(InnerProperty) };
		for (int32 i = 0; i < PropertyHelper.Num(); ++i)
		{
			if (StructProperty)
			{
				UScriptStruct* Struct{ StructProperty->Struct };

				UNameProperty* ID{ nullptr };
				UNumericProperty* Count{ nullptr };
				UNumericProperty* Index{ nullptr };
				UArrayProperty* InnerArrayProperty{ nullptr };

				for (UProperty* Prop = Struct->PropertyLink; Prop != nullptr; Prop = Prop->PropertyLinkNext)
				{
					if (Prop->GetName().StartsWith("ID"))
					{
						ID = Cast<UNameProperty>(Prop);
					}
					else if (Prop->GetName().StartsWith("Count"))
					{
						Count = Cast<UNumericProperty>(Prop);
					}
					else if (Prop->GetName().StartsWith("Index"))
					{
						Index = Cast<UNumericProperty>(Prop);
					}
				}

				uint8* ArrayPtr = PropertyHelper.GetRawPtr(i);

				if (ID)
				{
					FName* ID_Property = ID->ContainerPtrToValuePtr<FName>(ArrayPtr);

					if (!ID_Property || (*ID_Property).ToString().IsEmpty() || (*ID_Property).IsNone())
					{
						continue;
					}

					int32* CountValue = Count->ContainerPtrToValuePtr<int32>(ArrayPtr);
					int32* IndexValue = Index->ContainerPtrToValuePtr<int32>(ArrayPtr);

					FItemData Item;
					Item.ItemID = (*ID_Property).ToString();
					Item.Count = *CountValue;
					Item.Index = *IndexValue;
					Items.Add(Item);
				}

				/* Setting properties
				* Property->SetFloatingPointPropertyValue(theOwner)
				* Property->SetPropertyValue_InContainer(theOwner)
				* UNumericProperty vs. UIntProperty??!?!?!?!?
				*/

				
			}
		}
	}

	return Items;
}

void UVeritexMasterInventoryComponent::AddUserItem(const FString & ItemID, int32 Count)
{
	Server_AddUserItem(ItemID, Count);
}

void UVeritexMasterInventoryComponent::Server_AddUserItem_Implementation(const FString  & ItemID, int32 Count)
{
	AddPlayerItem(ItemID, Count, 0);
}

bool UVeritexMasterInventoryComponent::Server_AddUserItem_Validate(const FString  & ItemID, int32 Count)
{
	return true;
}

TArray<FItemData> UVeritexMasterInventoryComponent::GetInventoryItems()
{
	return InventoryItems;
}

TArray<FItemData> UVeritexMasterInventoryComponent::GetEquipmentItems()
{
	return EquipmentItems;
}

bool UVeritexMasterInventoryComponent::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	return Super::ProcessConsoleExec(Cmd, Ar, Executor);
}

// Called when the game starts
void UVeritexMasterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasInfiniteWeight())
	{
		MaxWeight = 2147483647; // Max Value of 32-bit float.
	}

	// ...

}


// Called every frame
void UVeritexMasterInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UVeritexMasterInventoryComponent::StartCookingTimer(const FString& UncookedItemID, const FString& CookedItemID, float CookingTime)
{
	FTimerDelegate CookingDelegateHandle;
	CookingDelegateHandle.BindUFunction(this, FName("DoCookingTimerCheck"), UncookedItemID, CookedItemID);
	GetWorld()->GetTimerManager().SetTimer(CookingTimer, CookingDelegateHandle, CookingTime, false);
}


