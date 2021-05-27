// Copyright Bruno Silva. All rights reserved.


#include "GASInventory.h"
#include <AbilitySystemComponent.h>
#include "Net/UnrealNetwork.h"

bool FItemSlot::CanSlotItem(AInventoryItem* NewItem) const
{
	if (!NewItem || !(NewItem->ItemData)) return true;

	bool bIsItemCategoryAllowed = AllowedItemCategories.HasTag(NewItem->ItemData->ItemCategory);
	bool bIsItemSizeAllowed = AllowedItemSizes.HasTag(NewItem->ItemData->ItemSize);

	return bIsItemCategoryAllowed && bIsItemSizeAllowed;
}

UItemData::UItemData()
{

}

FPrimaryAssetId UItemData::GetPrimaryAssetId() const
{
	return Super::GetPrimaryAssetId();
}

AInventoryItem* UItemData::CreateInventoryItem(UWorld* World, UItemData* ItemData)
{
	if (World && ItemData && ItemData->InventoryItemClass)
	{
		UClass* ItemClass = ItemData->InventoryItemClass;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AInventoryItem* NewItem = World->SpawnActor<AInventoryItem>(ItemClass, SpawnParams);
		if (NewItem)
		{
			NewItem->SetItemData(ItemData);
			return NewItem;
		}
	}
	return nullptr;
}

AInventoryItem::AInventoryItem()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInventoryItem, ItemData);
	DOREPLIFETIME(AInventoryItem, OwnerInventoryItem);
	DOREPLIFETIME(AInventoryItem, OwnerSlotName);
	DOREPLIFETIME(AInventoryItem, bIsItemActive);
	DOREPLIFETIME(AInventoryItem, bIsItemVisible);
	DOREPLIFETIME(AInventoryItem, ItemSlots);
}

void AInventoryItem::SetItemData(UItemData* NewSourceItem)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// Source item should only be set once.
		if (!ItemData)
		{
			ItemData = NewSourceItem;
			if (ItemData)
			{
				ItemSlots.Append(ItemData->ItemSlots);
			}
		}
	}
}

void AInventoryItem::SetOwnerASC(UAbilitySystemComponent* NewASC)
{
	check(ItemData);

	if (GetLocalRole() == ROLE_Authority && OwnerASC != NewASC)
	{
		if (OwnerASC)
		{
			RemoveAbilitiesFromASC(ActiveAbilitiesHandles);
			RemoveAbilitiesFromASC(PassiveAbilitiesHandles);
			RemoveEffectsFromASC(ActiveEffectsHandles);
			RemoveEffectsFromASC(PassiveEffectsHandles);
		}
		OwnerASC = NewASC;
		if (OwnerASC)
		{
			GiveAbilitiesToASC(ItemData->PassiveAbilities, PassiveAbilitiesHandles);
			ApplyEffectsToASC(ItemData->PassiveEffects, PassiveEffectsHandles);
		}

		// Update ASC on items in inventory.
		for (FItemSlot& Slot : ItemSlots)
		{
			for (AInventoryItem* Item : Slot.InventoryItems)
			{
				Item->SetOwnerASC(NewASC);
			}
		}
	}
}

void AInventoryItem::SetOwnerItem(AInventoryItem* NewOwner, FName SlotName)
{
	OwnerInventoryItem = NewOwner;
	OwnerSlotName = SlotName;
	if (OwnerInventoryItem)
	{
		USceneComponent* AttachComponent = OwnerInventoryItem->GetAttachToComponent();
		AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, SlotName);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

USceneComponent* AInventoryItem::GetAttachToComponent_Implementation() const
{
	return AlternativeAttachComponent ? AlternativeAttachComponent : GetRootComponent();
}

void AInventoryItem::ActivateItem()
{
	if (bIsItemActive || GetLocalRole() < ROLE_Authority) return;

	bIsItemActive = true;
	if (OwnerASC)
	{
		GiveAbilitiesToASC(ItemData->ActiveAbilities, ActiveAbilitiesHandles);
		ApplyEffectsToASC(ItemData->ActiveEffects, ActiveEffectsHandles);
	}

	BP_OnActivateItem();

	for (FItemSlot& Slot : ItemSlots)
	{
		if (Slot.bActivateItem)
		{
			for (AInventoryItem* Item : Slot.InventoryItems)
			{
				Item->ActivateItem();
			}
		}
	}
}

void AInventoryItem::DeactivateItem()
{
	if (!bIsItemActive || GetLocalRole() < ROLE_Authority) return;

	bIsItemActive = false;
	if (OwnerASC)
	{
		RemoveAbilitiesFromASC(ActiveAbilitiesHandles);
		RemoveEffectsFromASC(ActiveEffectsHandles);
	}

	BP_OnDeactivateItem();

	for (FItemSlot& Slot : ItemSlots)
	{
		for (AInventoryItem* Item : Slot.InventoryItems)
		{
			Item->DeactivateItem();
		}
	}
}

void AInventoryItem::ShowItem()
{
	if (bIsItemVisible) return;

	bIsItemVisible = true;
	if (GetRootComponent())
	{
		GetRootComponent()->SetHiddenInGame(false);
	}

	BP_OnShowItem();

	for (FItemSlot& Slot : ItemSlots)
	{
		if (Slot.bShowItem)
		{
			for (AInventoryItem* Item : Slot.InventoryItems)
			{
				Item->ShowItem();
			}
		}
	}
}

void AInventoryItem::HideItem()
{
	if (!bIsItemVisible) return;

	bIsItemVisible = false;
	if (GetRootComponent())
	{
		GetRootComponent()->SetHiddenInGame(true);
	}

	BP_OnHideItem();

	for (FItemSlot& Slot : ItemSlots)
	{
		for (AInventoryItem* Item : Slot.InventoryItems)
		{
			Item->HideItem();
		}
	}
}

void AInventoryItem::SetItemEnabled(bool bEnable, UAbilitySystemComponent* NewOwnerASC)
{
	if (bEnable)
	{
		SetOwnerASC(NewOwnerASC);
		ActivateItem();
		ShowItem();
	}
	else
	{
		HideItem();
		DeactivateItem();
		SetOwnerASC(nullptr);
	}
}

int AInventoryItem::RemoveAbilitiesFromASC(TArray<FGameplayAbilitySpecHandle>& InAbilityHandles)
{
	if (!OwnerASC || GetLocalRole() < ROLE_Authority) return 0;

	int NumAbilitiesRemoved = 0;
	for (const FGameplayAbilitySpecHandle& AbilityHandle : InAbilityHandles)
	{
		OwnerASC->ClearAbility(AbilityHandle);
		NumAbilitiesRemoved++;
	}

	return NumAbilitiesRemoved;
}

int AInventoryItem::RemoveEffectsFromASC(TArray<FActiveGameplayEffectHandle>& InEffectHandles)
{
	if (!OwnerASC || GetLocalRole() < ROLE_Authority) return 0;

	int NumEffectsRemoved = 0;
	for (const FActiveGameplayEffectHandle& EffectHandle : InEffectHandles)
	{
		OwnerASC->RemoveActiveGameplayEffect(EffectHandle);
		NumEffectsRemoved++;
	}

	return NumEffectsRemoved;
}

void AInventoryItem::GiveAbilitiesToASC(TArray<TSubclassOf<UGameplayAbility>>& InAbilities, TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles)
{
	if (!OwnerASC || GetLocalRole() < ROLE_Authority) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : InAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass);
		OutAbilityHandles.Add(OwnerASC->GiveAbility(AbilitySpec));
	}
}

void AInventoryItem::ApplyEffectsToASC(TArray<TSubclassOf<UGameplayEffect>>& InEffects, TArray<FActiveGameplayEffectHandle>& OutEffectHandles)
{
	if (!OwnerASC || GetLocalRole() < ROLE_Authority) return;

	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	for (const TSubclassOf<UGameplayEffect>& EffectClass : InEffects)
	{
		UGameplayEffect* Effect = EffectClass->GetDefaultObject<UGameplayEffect>();
		OutEffectHandles.Add(OwnerASC->ApplyGameplayEffectToSelf(Effect, 1.0f, EffectContext));
	}
}

bool AInventoryItem::AddItem(AInventoryItem* NewItem)
{
	if (GetLocalRole() < ROLE_Authority || !NewItem) return false;

	FItemSlot* ItemSlot = FindSlotForItem(NewItem);
	if (ItemSlot)
	{
		return AddItemToSlot(NewItem, ItemSlot->SlotName);
	}
	return false;
}

bool AInventoryItem::AddItemToSlot(AInventoryItem* NewItem, FName SlotName)
{
	if (GetLocalRole() < ROLE_Authority || !NewItem) return false;

	FItemSlot* ItemSlot = FindSlotByName(SlotName);
	if (ItemSlot && CanPlaceItemInSlot(NewItem, SlotName))
	{
		ItemSlot->InventoryItems.AddUnique(NewItem);
		NewItem->SetOwnerASC(OwnerASC);
		NewItem->SetOwnerItem(this, SlotName);
		bIsItemActive && ItemSlot->bActivateItem ? NewItem->ActivateItem() : NewItem->DeactivateItem();
		bIsItemVisible && ItemSlot->bShowItem ? NewItem->ShowItem() : NewItem->HideItem();

		OnItemAdded.Broadcast(NewItem, ItemSlot->SlotName);
		OnInventoryChanged.Broadcast(nullptr, FName(""));
		return true;
	}
	return false;
}

bool AInventoryItem::RemoveItem(AInventoryItem* ItemToRemove)
{
	if (GetLocalRole() < ROLE_Authority || !ItemToRemove) return false;
	return RemoveItemFromSlot(ItemToRemove, ItemToRemove->OwnerSlotName);
}

bool AInventoryItem::RemoveItemFromSlot(AInventoryItem* ItemToRemove, FName SlotName)
{
	if (GetLocalRole() < ROLE_Authority || !ItemToRemove) return false;

	FItemSlot* ItemSlot = FindSlotByName(SlotName);
	if (ItemSlot)
	{
		ItemSlot->InventoryItems.Remove(ItemToRemove);
		ItemToRemove->DeactivateItem();
		ItemToRemove->HideItem();
		ItemToRemove->SetOwnerItem(nullptr, FName(""));

		OnItemRemoved.Broadcast(ItemToRemove, ItemSlot->SlotName);
		OnInventoryChanged.Broadcast(nullptr, FName(""));
		return true;
	}
	return false;
}

FItemSlot* AInventoryItem::FindSlotForItem(AInventoryItem* NewItem)
{
	for (FItemSlot& ItemSlot : ItemSlots)
	{
		if (CanPlaceItemInSlot(NewItem, ItemSlot.SlotName))
		{
			return &ItemSlot;
		}
	}
	return nullptr;
}

FItemSlot* AInventoryItem::FindSlotByName(FName SlotName)
{
	FItemSlot* ItemSlot = ItemSlots.FindByPredicate([SlotName](const FItemSlot& Slot)
	{
		return Slot.SlotName.IsEqual(SlotName);
	});
	return ItemSlot;
}

bool AInventoryItem::CanPlaceItemInSlot(AInventoryItem* NewItem, FName SlotName)
{
	FItemSlot* ItemSlot = FindSlotByName(SlotName);
	if (ItemSlot && ItemSlot->CanSlotItem(NewItem))
	{
		if (ItemSlot->InventoryItems.Contains(NewItem))
		{
			// Trying to re-add an item. We should allow this.
			return true;
		}

		int NumItemsInSlot = ItemSlot->InventoryItems.Num();
		if (NumItemsInSlot < ItemSlot->ItemCapacity)
		{
			return true;
		}
	}
	return false;
}

void AInventoryItem::GetInventoryItems(bool bIncludeSelf, bool bPropagateToChildren, TArray<AInventoryItem*>& OutItems)
{
	if (bIncludeSelf)
	{
		OutItems.Add(this);
	}

	for (FItemSlot& Slot : ItemSlots)
	{
		for (AInventoryItem* Item : Slot.InventoryItems)
		{
			OutItems.Add(Item);
			if (bPropagateToChildren)
			{
				Item->GetInventoryItems(false, true, OutItems);
			}
		}
	}
}

void AInventoryItem::BP_FilterItemsBySlot(FName SlotName, TArray<AInventoryItem*> InItems, TArray<AInventoryItem*>& OutItems)
{
	for (AInventoryItem* Item : InItems)
	{
		if (Item->OwnerSlotName.IsEqual(SlotName))
		{
			OutItems.Add(Item);
		}
	}
}

void AInventoryItem::SetAlternativeAttachToComponent(USceneComponent* NewComponent)
{
	AlternativeAttachComponent = NewComponent;

	// Reattach items to new component.
	for (FItemSlot& Slot : ItemSlots)
	{
		for (AInventoryItem* Item : Slot.InventoryItems)
		{
			Item->SetOwnerItem(this, Slot.SlotName);
		}
	}
}

void AInventoryItem::OnRep_ItemSlots()
{
	OnInventoryChanged.Broadcast(nullptr, FName(""));
}
