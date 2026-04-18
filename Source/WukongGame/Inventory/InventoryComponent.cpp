#include "Inventory/InventoryComponent.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/Actor.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(UWukongItem* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0) return false;

	FInventorySlot* Existing = FindSlot(Item);
	if (Existing)
	{
		int32 Space = Item->MaxStackSize - Existing->Quantity;
		Existing->Quantity += FMath::Min(Quantity, Space);
		OnItemAdded.Broadcast(*Existing);
		return true;
	}

	if (Slots.Num() >= MaxSlots) return false;

	FInventorySlot NewSlot;
	NewSlot.Item = Item;
	NewSlot.Quantity = FMath::Min(Quantity, Item->MaxStackSize);
	Slots.Add(NewSlot);
	OnItemAdded.Broadcast(NewSlot);
	return true;
}

bool UInventoryComponent::UseItem(UWukongItem* Item)
{
	if (!Item) return false;

	FInventorySlot* Slot = FindSlot(Item);
	if (!Slot || Slot->Quantity <= 0) return false;

	ApplyItemEffect(Item);
	Slot->Quantity--;

	int32 Remaining = Slot->Quantity;
	if (Remaining <= 0)
	{
		Slots.RemoveSingleSwap(*Slot);
	}

	OnItemUsed.Broadcast(Item, Remaining);
	return true;
}

bool UInventoryComponent::UseItemByID(FName ItemID)
{
	FInventorySlot* Slot = FindSlotByID(ItemID);
	if (!Slot) return false;
	return UseItem(Slot->Item);
}

UWukongItem* UInventoryComponent::StealConsumableItem()
{
	for (FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.Item->bBossCanSteal &&
			Slot.Item->ItemType == EItemType::Consumable)
		{
			UWukongItem* Stolen = Slot.Item;
			Slot.Quantity--;
			if (Slot.Quantity <= 0)
			{
				Slots.RemoveSingleSwap(Slot);
			}
			OnItemStolen.Broadcast(Stolen);
			return Stolen;
		}
	}
	return nullptr;
}

void UInventoryComponent::OpenInventory()
{
	bIsOpen = true;
	OnInventoryOpened.Broadcast();
}

void UInventoryComponent::CloseInventory()
{
	bIsOpen = false;
	OnInventoryClosed.Broadcast();
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	for (const FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.Item->ItemID == ItemID)
			return Slot.Quantity;
	}
	return 0;
}

void UInventoryComponent::ApplyItemEffect(UWukongItem* Item)
{
	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats) return;

	switch (Item->Effect)
	{
	case EItemEffect::HealHP:      Stats->Heal(Item->EffectValue);          break;
	case EItemEffect::HealStamina: Stats->ConsumeStamina(-Item->EffectValue); break;
	case EItemEffect::HealMana:    Stats->RestoreMana(Item->EffectValue);   break;
	default: break;
	}
}

FInventorySlot* UInventoryComponent::FindSlot(UWukongItem* Item)
{
	for (FInventorySlot& Slot : Slots)
	{
		if (Slot.Item == Item) return &Slot;
	}
	return nullptr;
}

FInventorySlot* UInventoryComponent::FindSlotByID(FName ItemID)
{
	for (FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.Item->ItemID == ItemID) return &Slot;
	}
	return nullptr;
}
