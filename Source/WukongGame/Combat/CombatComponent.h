#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle        UMETA(DisplayName = "대기"),
	Attacking   UMETA(DisplayName = "공격 중"),
	Dodging     UMETA(DisplayName = "회피 중"),
	Parrying    UMETA(DisplayName = "패리 중"),
	Staggered   UMETA(DisplayName = "경직")
};

USTRUCT(BlueprintType)
struct FComboAttack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName AnimationTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StaminaCost = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ComboWindowDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsFinisher = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboPerformed, int32, ComboIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDodge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitDealt, AActor*, Target, float, Damage);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnComboPerformed OnComboPerformed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnParrySuccess OnParrySuccess;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDodge OnDodge;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHitDealt OnHitDealt;

	// --- Actions ---
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryDodge(FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartParry();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndParry();

	// 애니메이션 노티파이에서 호출
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableWeaponTrace();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DisableWeaponTrace();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackAnimEnd();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyStagger(float Duration);

	UFUNCTION(BlueprintPure, Category = "Combat")
	ECombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsParrying() const { return CombatState == ECombatState::Parrying; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentComboIndex() const { return CurrentComboIndex; }

	// --- Config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Combo")
	TArray<FComboAttack> ComboChain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DodgeStaminaCost = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DodgeImpulse = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ParryWindowDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StaggerRecoveryTime = 1.5f;

private:
	ECombatState CombatState = ECombatState::Idle;
	int32 CurrentComboIndex = 0;
	bool bComboWindowOpen = false;
	bool bAttackQueued = false;
	bool bWeaponTraceActive = false;
	float StaggerTimer = 0.f;
	float ParryTimer = 0.f;

	UPROPERTY()
	TSet<AActor*> HitActorsThisSwing;

	void ResetCombo();
	void AdvanceCombo();
	void PerformWeaponTrace();
	void SetState(ECombatState NewState);
};
