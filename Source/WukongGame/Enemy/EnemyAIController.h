#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Enemy/EnemyCharacter.h"
#include "EnemyAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

namespace BBKeys
{
	static const FName TargetActor    = TEXT("TargetActor");
	static const FName PatrolLocation = TEXT("PatrolLocation");
	static const FName EnemyState     = TEXT("EnemyState");
	static const FName DistToTarget   = TEXT("DistToTarget");
	static const FName bCanAttack     = TEXT("bCanAttack");
	static const FName bIsStaggered   = TEXT("bIsStaggered");
}

UCLASS()
class WUKONGGAME_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearTarget();

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetCurrentTarget() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float PeripheralVisionAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float HearingRange = 800.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTreeComponent* BehaviorTreeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* PerceptionComp;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
	UPROPERTY()
	AEnemyCharacter* OwnerEnemy;

	float BBUpdateTimer = 0.f;
	static constexpr float BBUpdateRate = 0.1f;

	void UpdateBlackboard();
	void SetupPerception();
};
