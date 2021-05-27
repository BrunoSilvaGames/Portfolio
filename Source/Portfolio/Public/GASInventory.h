// Copyright Bruno Silva. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include <Abilities/GameplayAbility.h>
#include <GameplayEffect.h>
#include <GameplayTagContainer.h>
#include <GameplayAbilitySpec.h>
#include <GameplayEffectTypes.h>
#include "GASInventory.generated.h"

// Forward Declarations:
class UAbilitySystemComponent;
class AInventoryItem;
class UAbilitySystemComponent;

// Delegates:
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChangedSignature, AInventoryItem*, InventoryItem, FName, SlotName);

USTRUCT(BlueprintType)
struct FItemSlot
{
	GENERATED_BODY()

public:
	/** Constructor. */
	FItemSlot()
	{
		ItemCapacity = 1;
		bActivateItem = false;
		bShowItem = false;
	};

public:

	// Is the item category and size valid for the slot.
	bool CanSlotItem(AInventoryItem* NewItem) const;

	bool operator==(const FItemSlot& Other) const
	{
		return SlotName == Other.SlotName;
	};

	bool operator!=(const FItemSlot& Other) const
	{
		return !(*this == Other);
	};

	/** For use in TMap/TSet. */
	friend inline uint32 GetTypeHash(const FItemSlot& Key)
	{
		uint32 Hash = 0;
		Hash = HashCombine(Hash, GetTypeHash(Key.SlotName));
		return Hash;
	};

public:

	/** Used as socket name in AttachToActor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	FName SlotName;

	/** Only items with matching tags can be slotted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	FGameplayTagContainer AllowedItemCategories;

	/** Only items with matching tags can be slotted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	FGameplayTagContainer AllowedItemSizes;

	/** How many items can be stored in this slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	int ItemCapacity;

	/** Are items in this slot allowed to be activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	bool bActivateItem;

	/** Are items in this slot allowed to be visible. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
	bool bShowItem;

public:

	/** Items assigned to this slot. */
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	TArray<AInventoryItem*> InventoryItems;
};

UCLASS()
class PORTFOLIO_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Constructor. */
	UItemData();

//------------------------------------------------------------------------
// METHODS
//------------------------------------------------------------------------

public:

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Helper function. Create an inventory item with the given item data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	static AInventoryItem* CreateInventoryItem(UWorld* World, UItemData* ItemData);

//------------------------------------------------------------------------
// PROPERTIES
//------------------------------------------------------------------------

public:

	/** Display name for this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemName;

	/** Can only enter slots that allow this category. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemCategory;

	/** Can only enter slots that allow this size. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemSize;

	/** Abilities granted when this item enters an inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Passive")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	/** Effects applied when this item enters an inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Passive")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;

	/** Abilities granted when item enters an active slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Active")
	TArray<TSubclassOf<UGameplayAbility>> ActiveAbilities;

	/** Effects applied when item enters active slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Active")
	TArray<TSubclassOf<UGameplayEffect>> ActiveEffects;

	/** Class of proxy to spawn for this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<AInventoryItem> InventoryItemClass;

	/** List of slots for additional items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TArray<FItemSlot> ItemSlots;
};

UCLASS()
class PORTFOLIO_API AInventoryItem : public AActor
{
	GENERATED_BODY()

public:
	/** Constructor. */
	AInventoryItem();

//------------------------------------------------------------------------
// METHODS
//------------------------------------------------------------------------

public:

	/** Replicate properties to clients. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	/** Saves pointer to source item. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void SetItemData(UItemData* NewSourceItem);

	/** Should be called when the item enters or leaves an inventory. Applies passive abilities and effects. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void SetOwnerASC(UAbilitySystemComponent* NewASC);

	/** Should be called when this item becomes a child of another item. Attaches self to said item. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void SetOwnerItem(AInventoryItem* NewOwner, FName SlotName);

	/** Get the component to which inventory items should attach to. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	USceneComponent* GetAttachToComponent() const;

	/** Applies active abilities and effects. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void ActivateItem();

	/** Removes active abilities and effects. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void DeactivateItem();

	/** Set the item as visible. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void ShowItem();

	/** Set the item as hidden. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void HideItem();

	/** Activate/deactivate and show/hide this item. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemEnabled(bool bEnable, UAbilitySystemComponent* NewOwnerASC);

public: // Blueprint Interface

	/** Called when item is activated. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void BP_OnActivateItem();

	/** Called when item is deactivated. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void BP_OnDeactivateItem();

	/** Called when item is shown. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void BP_OnShowItem();

	/** Called when item is hidden. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void BP_OnHideItem();

public:

	/** Remove list of abilities from owning ability system component. */
	int RemoveAbilitiesFromASC(TArray<FGameplayAbilitySpecHandle>& InAbilityHandles);

	/** Remove list of effects from owning ability system component. */
	int RemoveEffectsFromASC(TArray<FActiveGameplayEffectHandle>& InEffectHandles);

	/** Give list of abilities to owning ability system component. */
	void GiveAbilitiesToASC(TArray<TSubclassOf<UGameplayAbility>>& InAbilities, TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles);

	/** Apply list of effects to owning ability system component. */
	void ApplyEffectsToASC(TArray<TSubclassOf<UGameplayEffect>>& InEffects, TArray<FActiveGameplayEffectHandle>& OutEffectHandles);

public:

	/** Finds a slot for the item. Returns true if the item was added to the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(AInventoryItem* NewItem);

	/** Activate/deactivate and show/hide the new item, if necessary. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemToSlot(AInventoryItem* NewItem, FName SlotName);

	/** Finds the item and removes from slot. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(AInventoryItem* ItemToRemove);

	/** Deactivate and hide the item to remove. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemFromSlot(AInventoryItem* ItemToRemove, FName SlotName);

	/** Finds a slot that can fit the item. */
	FItemSlot* FindSlotForItem(AInventoryItem* NewItem);

	/** Finds a slot with the given name. */
	FItemSlot* FindSlotByName(FName SlotName);

	/** Check if item is valid for slot and if slot is not full. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanPlaceItemInSlot(AInventoryItem* NewItem, FName SlotName);

	/** Get all items in slots. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GetInventoryItems(bool bIncludeSelf, bool bPropagateToChildren, TArray<AInventoryItem*>& OutItems);

	/** Return only the items that are in the given slot. */
	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (DisplayName = "FilterItemsBySlot"))
	static void BP_FilterItemsBySlot(FName SlotName, TArray<AInventoryItem*> InItems, TArray<AInventoryItem*>& OutItems);

public:

	/** Update alternative attach-to component. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetAlternativeAttachToComponent(USceneComponent* NewComponent);

public:

	UFUNCTION()
	void OnRep_ItemSlots();

//------------------------------------------------------------------------
// PROPERTIES
//------------------------------------------------------------------------

public:

	/** Item that this proxy represents. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Item")
	UItemData* ItemData;

	/** Pointer to ability system component of actor that owns this item. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	UAbilitySystemComponent* OwnerASC;

	/** Pointer to item in which we are slotted. */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Item")
	AInventoryItem* OwnerInventoryItem;

	/** Name of the slot in which we are slotted. */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Item")
	FName OwnerSlotName;

public:

	/** Are abilities/effects active. */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Item")
	bool bIsItemActive;

	/** Is item visible/hidden. */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Item")
	bool bIsItemVisible;

	/** Handles for active "active" abilities. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TArray<FGameplayAbilitySpecHandle> ActiveAbilitiesHandles;

	/** Handles for active "active" effects. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TArray<FActiveGameplayEffectHandle> ActiveEffectsHandles;

	/** Handles for active "passive" abilities. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TArray<FGameplayAbilitySpecHandle> PassiveAbilitiesHandles;

	/** Handles for active "passive" effects. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TArray<FActiveGameplayEffectHandle> PassiveEffectsHandles;

public:

	/** Array of slots that this item has. Each slot can have its own items. */
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_ItemSlots, Category = "Inventory")
	TArray<FItemSlot> ItemSlots;

public:

	/** If set, child proxies will attach to this actor instead. */
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	USceneComponent* AlternativeAttachComponent;

public:

	/** Called when an item is added to the inventory. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChangedSignature OnItemAdded;

	/** Called when an item is removed from the inventory. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChangedSignature OnItemRemoved;

	/** Called when an item in the inventory is added or removed. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChangedSignature OnInventoryChanged;
};

