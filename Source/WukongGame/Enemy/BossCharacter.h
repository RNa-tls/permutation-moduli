#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyCharacter.h"
#include "BossCharacter.generated.h"

USTRUCT(BlueprintType)
struct FBossPhase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Phase")
	float HealthThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Phase")
	float AttackDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Phase")
	float SpeedMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Phase")
	bool bBerserkMode = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Phase")
	bool bUnlocksSpecialAttack = false;

	UPROPERTY(BlueprintReadOnly, Category = "Phase")
	bool bActivated = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpecialAttackReady);

UCLASS()
class WUKONGGAME_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ABossCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Die() override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpecialAttackReady OnSpecialAttackReady;

	// --- 페이즈 ---
	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsInBerserkMode() const;

	// --- 특수 공격 ---
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformSpecialAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformGroundSlam();

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformRangedProjectile(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformAreaSweep();

	// 보스 고유 ID (QuestManager에서 참조)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	FName BossID = TEXT("Boss_Unknown");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	TArray<FBossPhase> Phases;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float SpecialAttackCooldown = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float GroundSlamDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float GroundSlamRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float RangedDamage = 50.f;

private:
	int32 CurrentPhaseIndex = 0;
	float LastSpecialAttackTime = -999.f;
	float OriginalAttackDamage = 0.f;
	float OriginalMoveSpeed = 400.f;

	void CheckPhaseTransition();
	void ActivatePhase(int32 PhaseIndex);
	void ApplyPhaseModifiers(const FBossPhase& Phase);
};
