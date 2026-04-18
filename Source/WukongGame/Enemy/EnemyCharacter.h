#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UCharacterStatsComponent;
class UBehaviorTree;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle        UMETA(DisplayName = "대기"),
	Patrolling  UMETA(DisplayName = "순찰"),
	Chasing     UMETA(DisplayName = "추격"),
	Attacking   UMETA(DisplayName = "공격"),
	Staggered   UMETA(DisplayName = "경직"),
	Dead        UMETA(DisplayName = "사망")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyStateChanged, EEnemyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

UCLASS()
class WUKONGGAME_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyStateChanged OnEnemyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDied OnEnemyDied;

	// --- AI ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	int32 SpiritPointReward = 100;

	// --- State ---
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetEnemyState(EEnemyState NewState);

	UFUNCTION(BlueprintPure, Category = "AI")
	EEnemyState GetEnemyState() const { return EnemyState; }

	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsAlive() const;

	// --- Actions ---
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Die();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCharacterStatsComponent* StatsComponent;

private:
	EEnemyState EnemyState = EEnemyState::Idle;

	UFUNCTION()
	void OnDeathCallback();
};
