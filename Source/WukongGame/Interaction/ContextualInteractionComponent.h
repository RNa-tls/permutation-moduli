#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionEvent.h"
#include "ContextualInteractionComponent.generated.h"

class AWukongCharacter;
class ABossCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionTriggered,
	EPlayerAction, PlayerAction, EBossReaction, BossReaction);

/**
 * UContextualInteractionComponent
 *
 * 보스에 붙이는 컴포넌트. 플레이어 행동(EPlayerAction)을 입력으로 받아
 * 등록된 FInteractionTrigger 목록에서 확률/쿨다운/범위를 검사한 뒤
 * 해당하는 EBossReaction을 BossAIController로 전달한다.
 *
 * 보스마다 TArray<FInteractionTrigger>를 다르게 설정해
 * 다양한 반응 패턴을 디자인할 수 있다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UContextualInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UContextualInteractionComponent();

	virtual void BeginPlay() override;

	// --- 이벤트 ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInteractionTriggered OnInteractionTriggered;

	// --- 외부 진입점: BossAIController 또는 AIPerception이 호출 ---
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EvaluateTriggers(EPlayerAction PlayerAction);

	// --- 트리거 등록 ---
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterTrigger(FInteractionTrigger Trigger);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ClearTriggers();

	// --- 기본 트리거 세트 적용 (보스 Blueprint에서 호출) ---
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetupDefaultTriggers();

	// --- 상태 ---
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsReacting() const { return bIsReacting; }

	// 등록된 트리거 목록 (Blueprint에서도 편집 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	TArray<FInteractionTrigger> Triggers;

protected:
	UPROPERTY()
	ABossCharacter* OwnerBoss = nullptr;

	UPROPERTY()
	AWukongCharacter* CachedPlayer = nullptr;

private:
	bool bIsReacting = false;

	float DistanceToPlayer() const;
	bool RollChance(float Chance) const;
	void ExecuteReaction(EBossReaction Reaction);
	void FindPlayerAndBoss();
};
