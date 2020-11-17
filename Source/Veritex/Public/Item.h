// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VeritexStructs.h"

#include "Item.generated.h"

UCLASS()
class VERITEX_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
		void SetItemData();

	UFUNCTION(BlueprintImplementableEvent)
		void OnCPPSpawned();

	UPROPERTY(BlueprintReadWrite, Category = Item)
		FItemData ItemData;
	
	
};
