#include "Character/WukongAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWukongAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<AWukongCharacter>(TryGetPawnOwner());
}

void UWukongAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter) return;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		Speed = OwnerCharacter->GetVelocity().Size2D();
		bIsInAir = MoveComp->IsFalling();
		bIsAccelerating = MoveComp->GetCurrentAcceleration().SizeSquared() > 0.f;
	}

	WukongState = OwnerCharacter->GetWukongState();
	bIsDead = (WukongState == EWukongState::Dead);
	bIsCloudSurfing = (WukongState == EWukongState::CloudSurfing);

	if (UCombatComponent* Combat = OwnerCharacter->CombatComponent)
	{
		CombatState = Combat->GetCombatState();
		ComboIndex = Combat->GetCurrentComboIndex();
		bIsParrying = Combat->IsParrying();
	}
}
