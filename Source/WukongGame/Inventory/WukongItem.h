#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WukongItem.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Consumable  UMETA(DisplayName = "소비 아이템"),
	Equipment   UMETA(DisplayName = "장비"),
	KeyItem     UMETA(DisplayName = "중요 아이템"),
	Material    UMETA(DisplayName = "재료")
};

UENUM(BlueprintType)
enum class EItemEffect : uint8
{
	HealHP      UMETA(DisplayName = "HP 회복"),
	HealStamina UMETA(DisplayName = "스태미나 회복"),
	HealMana    UMETA(DisplayName = "마나 회복"),
	BuffAttack  UMETA(DisplayName = "공격력 강화"),
	BuffDefense UMETA(DisplayName = "방어력 강화"),
	None        UMETA(DisplayName = "없음")
};

UCLASS(BlueprintType)
class WUKONGGAME_API UWukongItem : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType = EItemType::Consumable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effect")
	EItemEffect Effect = EItemEffect::HealHP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effect")
	float EffectValue = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effect")
	float BuffDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxStackSize = 9;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float UseAnimationDuration = 1.5f;

	// 보스가 이 아이템을 빼앗을 수 있는지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Interaction")
	bool bBossCanSteal = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Interaction")
	float BossHealOnSteal = 0.f;
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	UWukongItem* Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;

	bool IsEmpty() const { return Item == nullptr || Quantity <= 0; }
};
