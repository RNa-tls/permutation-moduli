#pragma once

#include "CoreMinimal.h"
#include "InteractionEvent.generated.h"

// 플레이어 행동 분류 - 보스가 감지하고 반응할 수 있는 모든 행동
UENUM(BlueprintType)
enum class EPlayerAction : uint8
{
	None            UMETA(DisplayName = "없음"),
	UsePotion       UMETA(DisplayName = "포션 사용"),
	OpenInventory   UMETA(DisplayName = "인벤토리 열기"),
	ExtendStaff     UMETA(DisplayName = "여의봉 확장"),
	MountStaff      UMETA(DisplayName = "여의봉 탑승"),
	Dodging         UMETA(DisplayName = "회피 중"),
	Parrying        UMETA(DisplayName = "패리 시도"),
	Transforming    UMETA(DisplayName = "변신 중"),
	CloudSurfing    UMETA(DisplayName = "근두운 탑승")
};

// 보스의 반응 행동
UENUM(BlueprintType)
enum class EBossReaction : uint8
{
	None            UMETA(DisplayName = "없음"),
	StealPotion     UMETA(DisplayName = "포션 강탈 + 자가 회복"),
	GrabStaff       UMETA(DisplayName = "봉 잡고 흔들기 (탑승 중 추락)"),
	CounterDodge    UMETA(DisplayName = "회피 카운터 (발 밟기)"),
	BreakParry      UMETA(DisplayName = "패리 부수기 (강공격)"),
	AntiTransform   UMETA(DisplayName = "변신 방해 (기 흡수)"),
	CloudAntiAir    UMETA(DisplayName = "근두운 격추")
};

USTRUCT(BlueprintType)
struct FInteractionTrigger
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trigger")
	EPlayerAction TriggerAction = EPlayerAction::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trigger")
	EBossReaction BossReaction = EBossReaction::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trigger")
	float DetectionRadius = 1500.f;

	// 반응 확률 (0~1) - 매 번 반응하지 않을 수 있음
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trigger")
	float ReactionChance = 0.8f;

	// 반응 쿨다운 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trigger")
	float Cooldown = 10.f;

	UPROPERTY(BlueprintReadOnly, Category = "Trigger")
	float LastReactionTime = -999.f;

	bool IsOnCooldown(float CurrentTime) const
	{
		return (CurrentTime - LastReactionTime) < Cooldown;
	}
};
