
// SlotArea.h

// Interfaces to the cSlotArea class representing a contiguous area of slots in a UI window




#pragma once

#include "../Inventory.h"



class cWindow;
class cPlayer;
class cChestEntity;
class cDropSpenserEntity;
class cFurnaceEntity;
class cCraftingRecipe;





class cSlotArea
{
public:
	cSlotArea(int a_NumSlots, cWindow & a_ParentWindow);
	virtual ~cSlotArea() {}  // force a virtual destructor in all subclasses
	
	int GetNumSlots(void) const { return m_NumSlots; }
	
	/// Called to retrieve an item in the specified slot for the specified player. Must return a valid cItem.
	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const = 0;
	
	/// Called to set an item in the specified slot for the specified player
	virtual void SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) = 0;
	
	/// Called when a player clicks in the window. Parameters taken from the click packet.
	virtual void Clicked(cPlayer & a_Player, int a_SlotNum, eClickAction a_ClickAction, const cItem & a_ClickedItem);
	
	/// Called from Clicked when the action is a shiftclick (left or right)
	virtual void ShiftClicked(cPlayer & a_Player, int a_SlotNum, const cItem & a_ClickedItem);
	
	/// Called from Clicked when the action is a caDblClick
	virtual void DblClicked(cPlayer & a_Player, int a_SlotNum);
	
	/// Called when a new player opens the same parent window. The window already tracks the player. CS-locked.
	virtual void OnPlayerAdded(cPlayer & a_Player) {} ;
	
	/// Called when one of the players closes the parent window. The window already doesn't track the player. CS-locked.
	virtual void OnPlayerRemoved(cPlayer & a_Player) {} ;
	
	/** Called to store as much of a_ItemStack in the area as possible. a_ItemStack is modified to reflect the change.
	The default implementation searches each slot for available space and distributes the stack there.
	if a_ShouldApply is true, the changes are written into the slots;
	if a_ShouldApply is false, only a_ItemStack is modified to reflect the number of fits (for fit-testing purposes)
	If a_KeepEmptySlots is true, empty slots will be skipped and won't be filled
	*/
	virtual void DistributeStack(cItem & a_ItemStack, cPlayer & a_Player, bool a_ShouldApply, bool a_KeepEmptySlots);
	
	/// Called on DblClicking to collect all stackable items into hand.
	/// The items are accumulated in a_Dragging and removed from the slots immediately.
	/// If a_CollectFullStacks is false, slots with full stacks are skipped while collecting.
	/// Returns true if full stack has been collected in a_Dragging, false if there's space remaining to fill.
	virtual bool CollectItemsToHand(cItem & a_Dragging, cPlayer & a_Player, bool a_CollectFullStacks);
	
protected:
	int       m_NumSlots;
	cWindow & m_ParentWindow;
} ;





/// Handles any part of the inventory, using parameters in constructor to distinguish between the parts
class cSlotAreaInventoryBase :
	public cSlotArea
{
	typedef cSlotArea super;
	
public:
	cSlotAreaInventoryBase(int a_NumSlots, int a_SlotOffset, cWindow & a_ParentWindow);
	
	// Creative inventory's click handling is somewhat different from survival inventory's, handle that here:
	virtual void Clicked(cPlayer & a_Player, int a_SlotNum, eClickAction a_ClickAction, const cItem & a_ClickedItem) override;

	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;
	
protected:
	int m_SlotOffset;  // Index that this area's slot 0 has in the underlying cInventory
} ;





/// Handles the main inventory of each player, excluding the armor and hotbar
class cSlotAreaInventory :
	public cSlotAreaInventoryBase
{
	typedef cSlotAreaInventoryBase super;
	
public:
	cSlotAreaInventory(cWindow & a_ParentWindow) :
		cSlotAreaInventoryBase(cInventory::invInventoryCount, cInventory::invInventoryOffset, a_ParentWindow)
	{
	}
} ;





/// Handles the hotbar of each player
class cSlotAreaHotBar :
	public cSlotAreaInventoryBase
{
	typedef cSlotAreaInventoryBase super;
	
public:
	cSlotAreaHotBar(cWindow & a_ParentWindow)	:
		cSlotAreaInventoryBase(cInventory::invHotbarCount, cInventory::invHotbarOffset, a_ParentWindow)
	{
	}
} ;





/// Handles the armor area of the player's inventory
class cSlotAreaArmor :
	public cSlotAreaInventoryBase
{
public:
	cSlotAreaArmor(cWindow & a_ParentWindow)	:
		cSlotAreaInventoryBase(cInventory::invArmorCount, cInventory::invArmorOffset, a_ParentWindow)
	{
	}

	// Distributing the stack is allowed only for compatible items (helmets into helmet slot etc.)
	virtual void DistributeStack(cItem & a_ItemStack, cPlayer & a_Player, bool a_ShouldApply, bool a_KeepEmptySlots) override;
} ;





/// Handles any slot area that is representing a cItemGrid; same items for all the players
class cSlotAreaItemGrid :
	public cSlotArea,
	public cItemGrid::cListener
{
	typedef cSlotArea super;
	
public:
	cSlotAreaItemGrid(cItemGrid & a_ItemGrid, cWindow & a_ParentWindow);
	
	virtual ~cSlotAreaItemGrid();
	
	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;

protected:
	cItemGrid & m_ItemGrid;
	
	// cItemGrid::cListener overrides:
	virtual void OnSlotChanged(cItemGrid * a_ItemGrid, int a_SlotNum) override;
} ;





/** A cSlotArea with items layout that is private to each player and is temporary, such as
a crafting grid or an enchantment table.
This common ancestor stores the items in a per-player map. It also implements tossing items from the map.
*/
class cSlotAreaTemporary :
	public cSlotArea
{
	typedef cSlotArea super;
	
public:
	cSlotAreaTemporary(int a_NumSlots, cWindow & a_ParentWindow);
	
	// cSlotArea overrides:
	virtual const cItem * GetSlot        (int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot        (int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;
	virtual void          OnPlayerAdded  (cPlayer & a_Player) override;
	virtual void          OnPlayerRemoved(cPlayer & a_Player) override;
	
	/// Tosses the player's items in slots [a_Begin, a_End) (ie. incl. a_Begin, but excl. a_End)
	void TossItems(cPlayer & a_Player, int a_Begin, int a_End);
	
protected:
	typedef std::map<int, std::vector<cItem> > cItemMap;  // Maps EntityID -> items
	
	cItemMap m_Items;
	
	/// Returns the pointer to the slot array for the player specified.
	cItem * GetPlayerSlots(cPlayer & a_Player);
} ;





class cSlotAreaCrafting :
	public cSlotAreaTemporary
{
	typedef cSlotAreaTemporary super;
	
public:
	/// a_GridSize is allowed to be only 2 or 3
	cSlotAreaCrafting(int a_GridSize, cWindow & a_ParentWindow);

	// cSlotAreaTemporary overrides:
	virtual void Clicked        (cPlayer & a_Player, int a_SlotNum, eClickAction a_ClickAction, const cItem & a_ClickedItem) override;
	virtual void DblClicked     (cPlayer & a_Player, int a_SlotNum);
	virtual void OnPlayerRemoved(cPlayer & a_Player) override;
	
	// Distributing items into this area is completely disabled
	virtual void DistributeStack(cItem & a_ItemStack, cPlayer & a_Player, bool a_ShouldApply, bool a_KeepEmptySlots) override {}

protected:
	/// Maps player's EntityID -> current recipe; not a std::map because cCraftingGrid needs proper constructor params
	typedef std::list<std::pair<int, cCraftingRecipe> > cRecipeMap;
	
	int        m_GridSize;
	cRecipeMap m_Recipes;
	
	/// Handles a click in the result slot. Crafts using the current recipe, if possible
	void ClickedResult(cPlayer & a_Player);
	
	/// Handles a shift-click in the result slot. Crafts using the current recipe until it changes or no more space for result.
	void ShiftClickedResult(cPlayer & a_Player);
	
	/// Updates the current recipe and result slot based on the ingredients currently in the crafting grid of the specified player
	void UpdateRecipe(cPlayer & a_Player);
	
	/// Retrieves the recipe for the specified player from the map, or creates one if not found
	cCraftingRecipe & GetRecipeForPlayer(cPlayer & a_Player);
} ;





class cSlotAreaChest :
	public cSlotArea
{
public:
	cSlotAreaChest(cChestEntity * a_Chest, cWindow & a_ParentWindow);
	
	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;
	
protected:
	cChestEntity * m_Chest;
} ;





class cSlotAreaDoubleChest :
	public cSlotArea
{
public:
	cSlotAreaDoubleChest(cChestEntity * a_TopChest, cChestEntity * a_BottomChest, cWindow & a_ParentWindow);
	
	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;
	
protected:
	cChestEntity * m_TopChest;
	cChestEntity * m_BottomChest;
} ;





class cSlotAreaFurnace :
	public cSlotArea,
	public cItemGrid::cListener
{
	typedef cSlotArea super;
	
public:
	cSlotAreaFurnace(cFurnaceEntity * a_Furnace, cWindow & a_ParentWindow);
	
	virtual ~cSlotAreaFurnace();
	
	virtual void          Clicked(cPlayer & a_Player, int a_SlotNum, eClickAction a_ClickAction, const cItem & a_ClickedItem) override;
	virtual const cItem * GetSlot(int a_SlotNum, cPlayer & a_Player) const override;
	virtual void          SetSlot(int a_SlotNum, cPlayer & a_Player, const cItem & a_Item) override;
	
protected:
	cFurnaceEntity * m_Furnace;

	// cItemGrid::cListener overrides:
	virtual void OnSlotChanged(cItemGrid * a_ItemGrid, int a_SlotNum) override;
} ;



