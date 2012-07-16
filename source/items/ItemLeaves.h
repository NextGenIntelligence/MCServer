#pragma once

#include "Item.h"


class cItemLeavesHandler : public cItemHandler
{
public:
	cItemLeavesHandler(int a_ItemID)
		: cItemHandler(a_ItemID)
	{
	}
	virtual NIBBLETYPE GetBlockMeta(NIBBLETYPE a_ItemMeta) override
	{
		return a_ItemMeta | 0x4;	//0x4 bit set means this is a player places leave
	}
};