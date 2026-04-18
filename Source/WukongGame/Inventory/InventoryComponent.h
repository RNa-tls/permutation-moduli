#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/WukongItem.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, UWukongItem*, Item, int32, RemainingCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, FInventorySlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemStolen, UWukongItem*, StolenItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClosed);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// --- Delegates (보스가 구독) ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemStolen OnItemStolen;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryOpened OnInventoryOpened;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryClosed OnInventoryClosed;

	// --- Actions ---
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UWukongItem* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(UWukongItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItemByID(FName ItemID);

	// 보스가 아이템을 강탈할 때 호출 (BossAIController에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UWukongItem* StealConsumableItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsInventoryOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID) const { return GetItemCount(ItemID) > 0; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FInventorySlot> GetAllSlots() const { return Slots; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots = 20;

private:
	UPROPERTY()
	TArray<FInventorySlot> Slots;

	bool bIsOpen = false;

	FInventorySlot* FindSlot(UWukongItem* Item);
	FInventorySlot* FindSlotByID(FName ItemID);
	void ApplyItemEffect(UWukongItem* Item);
};
