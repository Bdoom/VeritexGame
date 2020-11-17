// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "VeritexAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FVeritexAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FVeritexAnimInstanceProxy();

	FVeritexAnimInstanceProxy(UAnimInstance* Instance);

	virtual void Update(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadWrite)
		float StateWight;

};

UCLASS(Transient, Blueprintable)
class VERITEX_API UVeritexAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;

	FVeritexAnimInstanceProxy Proxy;

	UPROPERTY(BlueprintReadWrite)
		APawn* OwningPawn;



protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

	friend struct FVeritexAnimInstanceProxy;
};
