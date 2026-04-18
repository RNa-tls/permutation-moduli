#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Combat/CombatComponent.h"
#include "Character/WukongCharacter.h"
#include "WukongAnimInstance.generated.h"

UCLASS()
class WUKONGGAME_API UWukongAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Locomotion ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsAccelerating = false;

	// --- Combat ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	ECombatState CombatState = ECombatState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	int32 ComboIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsParrying = false;

	// --- Character State ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
	EWukongState WukongState = EWukongState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
	bool bIsCloudSurfing = false;

private:
	UPROPERTY()
	AWukongCharacter* OwnerCharacter;
};
