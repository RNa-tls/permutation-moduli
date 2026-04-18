#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyAIController.h"
#include "Interaction/InteractionEvent.h"
#include "BossAIController.generated.h"

class ABossCharacter;
class AWukongCharacter;
class UInventoryComponent;
class AStaffWeapon;
class UContextualInteractionComponent;

UCLASS()
class WUKONGGAME_API ABossAIController : public AEnemyAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// --- 플레이어 행동 반응 콜백 ---
	// InventoryComponent::OnItemUsed 에 바인딩
	UFUNCTION()
	void OnPlayerUsedItem(UWukongItem* Item, int32 RemainingCount);

	// InventoryComponent::OnInventoryOpened 에 바인딩
	UFUNCTION()
	void OnPlayerOpenedInventory();

	// StaffWeapon::OnStaffExtended 에 바인딩
	UFUNCTION()
	void OnStaffExtended(float ExtendedLength, FVector TipLocation);

	// StaffWeapon::OnStaffRetracted 에 바인딩
	UFUNCTION()
	void OnStaffRetracted();

	// CombatComponent::OnDodge 에 바인딩
	UFUNCTION()
	void OnPlayerDodged();

	// CombatComponent::OnComboPerformed 에 바인딩
	UFUNCTION()
	void OnPlayerCombo(int32 ComboIndex);

	// --- 보스 반응 행동 ---
	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void ReactStealPotion();

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void ReactGrabStaff();

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void ReactCounterDodge();

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void ReactBreakParry();

	// --- 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI")
	float StealPotionRange = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI")
	float GrabStaffRange = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI")
	float CounterDodgeRange = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI")
	float ReactionCooldown = 8.f;

protected:
	UPROPERTY()
	ABossCharacter* BossOwner = nullptr;

	UPROPERTY()
	AWukongCharacter* TrackedPlayer = nullptr;

	UPROPERTY()
	UContextualInteractionComponent* InteractionComp = nullptr;

private:
	float LastReactionTime = -999.f;

	bool CanReact() const;
	void MarkReacted();
	float DistanceToPlayer() const;

	void BindToPlayer(AWukongCharacter* Player);
	void UnbindFromPlayer(AWukongCharacter* Player);
};
