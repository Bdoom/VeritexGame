// Daniel Gleason (C) 2017

#include "VeritexAnimInstance.h"

void UVeritexAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningPawn)
		return;

}

void UVeritexAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

FAnimInstanceProxy* UVeritexAnimInstance::CreateAnimInstanceProxy()
{
	return &Proxy;
}

void UVeritexAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{

}

FVeritexAnimInstanceProxy::FVeritexAnimInstanceProxy(UAnimInstance* Instance)
{

}

FVeritexAnimInstanceProxy::FVeritexAnimInstanceProxy()
{

}

void FVeritexAnimInstanceProxy::Update(float DeltaSeconds)
{

}
