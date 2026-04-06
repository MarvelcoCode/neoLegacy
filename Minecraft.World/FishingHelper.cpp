
#include "../Minecraft.World/WeighedTreasure.h"
#include "../Minecraft.World/Biome.h"
#include "../Minecraft.World/FishingHelper.h"
#include "../Minecraft.World/ItemInstance.h"
#include "../Minecraft.World/EnchantmentHelper.h"
#include <memory>
#include "net.minecraft.world.item.h"

FishingHelper* FishingHelper::getInstance()
{
	static FishingHelper instance;
	return &instance;
}

FishingHelper::FishingHelper() : 
	random(new Random()), level0Array(3), level1Array(3), level2Array(3), level3Array(3),
      fishingFishArray(4), fishingJunkArray(12), fishingTreasuresArray(5)
{

	// Source: https://minecraft.wiki/w/Fishing
	level0Array[0] = new CatchTypeWeighedItem(CatchType::JUNK, 10 );
	level0Array[1] = new CatchTypeWeighedItem(CatchType::TREASURE, 5 );
	level0Array[2] = new CatchTypeWeighedItem(CatchType::FISH, 85 );

	level1Array[0] = new CatchTypeWeighedItem(CatchType::JUNK, 81 );
	level1Array[1] = new CatchTypeWeighedItem(CatchType::TREASURE, 71 );
	level1Array[2] = new CatchTypeWeighedItem(CatchType::FISH, 848 );

	level2Array[0] = new CatchTypeWeighedItem(CatchType::JUNK, 61 );
	level2Array[1] = new CatchTypeWeighedItem(CatchType::TREASURE, 92 );
	level2Array[2] = new CatchTypeWeighedItem(CatchType::FISH, 847 );

	level3Array[0] = new CatchTypeWeighedItem(CatchType::JUNK, 41 );
	level3Array[1] = new CatchTypeWeighedItem(CatchType::TREASURE, 113 );
	level3Array[2] = new CatchTypeWeighedItem(CatchType::FISH, 845 );

	fishingTreasuresArray[0] = new CatchWeighedItem(Item::bow_Id, 1, 0, 1);
	fishingTreasuresArray[1] = new CatchWeighedItem(Item::book_Id, 1, 0, 1);
	fishingTreasuresArray[2] = new CatchWeighedItem(Item::fishingRod_Id, 1, 0, 1);
	fishingTreasuresArray[3] = new CatchWeighedItem(Item::nameTag_Id, 1, 0, 2); // Doubled the chance of name tags to account for lack of nautilus shells.
	fishingTreasuresArray[4] = new CatchWeighedItem(Item::saddle_Id, 1, 0, 1);

	fishingFishArray[0] = new CatchWeighedItem(Item::fish_raw_Id, 1, 0, 60); // Fish
	fishingFishArray[1] = new CatchWeighedItem(Item::fish_raw_Id, 1, 1, 25); // Salmon
	fishingFishArray[2] = new CatchWeighedItem(Item::fish_raw_Id, 1, 2, 2); // Clownfish
	fishingFishArray[3] = new CatchWeighedItem(Item::fish_raw_Id, 1, 3, 13); // Pufferfish

	fishingJunkArray[0] = new CatchWeighedItem(Tile::waterLily_Id, 1, 0, 17);
	fishingJunkArray[1] = new CatchWeighedItem(Item::bone_Id, 1, 0, 10);
	fishingJunkArray[2] = new CatchWeighedItem(Item::bowl_Id, 1, 0, 10);
	fishingJunkArray[3] = new CatchWeighedItem(Item::leather_Id, 1, 0, 10);
	fishingJunkArray[4] = new CatchWeighedItem(Item::boots_leather_Id, 1, 0, 10);
	fishingJunkArray[5] = new CatchWeighedItem(Item::rotten_flesh_Id, 1, 0, 10);
	fishingJunkArray[6] = new CatchWeighedItem(Item::potion_Id, 1, 0, 10); // Water bottle
	fishingJunkArray[7] = new CatchWeighedItem(Tile::tripWireSource_Id, 1, 0, 10);
	fishingJunkArray[8] = new CatchWeighedItem(Item::stick_Id, 1, 0, 5);
	fishingJunkArray[9] = new CatchWeighedItem(Item::string_Id, 1, 0, 5);
	fishingJunkArray[10] = new CatchWeighedItem(Item::fishingRod_Id, 1, 0, 2);
	fishingJunkArray[11] = new CatchWeighedItem(Item::dye_powder_Id, 10, 0, 1); // 10 ink sacs
}

CatchType FishingHelper::getRandCatchType(int level)
{
	CatchTypeWeighedItem* catchTypeWeighedItem = nullptr;
	switch (level) {
		case 0:
			catchTypeWeighedItem = static_cast<CatchTypeWeighedItem *>(WeighedRandom::getRandomItem(random, level0Array));
			return catchTypeWeighedItem->getType();
		case 1:
			catchTypeWeighedItem = static_cast<CatchTypeWeighedItem *>(WeighedRandom::getRandomItem(random, level1Array));
			return catchTypeWeighedItem->getType();
		case 2:
			catchTypeWeighedItem = static_cast<CatchTypeWeighedItem *>(WeighedRandom::getRandomItem(random, level2Array));
			return catchTypeWeighedItem->getType();
		case 3:
			catchTypeWeighedItem = static_cast<CatchTypeWeighedItem *>(WeighedRandom::getRandomItem(random, level3Array));
			return catchTypeWeighedItem->getType();
	}
}

CatchWeighedItem* FishingHelper::getRandCatch(CatchType catchType)
{
	CatchWeighedItem* catchWeighedItem = nullptr;
	switch (catchType) {
		case CatchType::FISH:
			return static_cast<CatchWeighedItem *>(WeighedRandom::getRandomItem(random, fishingFishArray));
		case CatchType::TREASURE:
			return static_cast<CatchWeighedItem *>(WeighedRandom::getRandomItem(random, fishingTreasuresArray));;
		case CatchType::JUNK:
			return static_cast<CatchWeighedItem *>(WeighedRandom::getRandomItem(random, fishingJunkArray));
	}
}

std::shared_ptr<ItemInstance> FishingHelper::handleCatch(CatchWeighedItem* weighedCatch, CatchType catchType)
{	
	std::shared_ptr<ItemInstance> itemInstance = std::make_shared<ItemInstance>(
		weighedCatch->getItemId(), weighedCatch->getCount(), weighedCatch->getAuxValue()
	);
	
	if ((itemInstance->id == Item::fishingRod_Id && catchType == CatchType::JUNK) || (itemInstance->id == Item::boots_leather_Id)) {
		itemInstance->setAuxValue((int) (itemInstance->getMaxDamage() * ((double) random->nextInt(901) + 100.0) / 1000.0)); // 10% to 100% damage 
	}
	else if (itemInstance->id == Item::fishingRod_Id && catchType == CatchType::TREASURE) {
		itemInstance->setAuxValue((int) (itemInstance->getMaxDamage() * ((double) random->nextInt(251) + 750.0) / 1000.0)); // 75% to 100% damage
		itemInstance = EnchantmentHelper::enchantItem(random, itemInstance, random->nextInt(9) + 22); // 22 to 30 enchantment level
	}
	else if (itemInstance->id == Item::bow_Id) {
		itemInstance->setAuxValue((int) (itemInstance->getMaxDamage() * ((double) random->nextInt(251) + 750.0) / 1000.0)); // 75% to 100% damage
		itemInstance = EnchantmentHelper::enchantItem(random, itemInstance, random->nextInt(9) + 22); // 22 to 30 enchantment level
	}
	else if (itemInstance->id == Item::book_Id) {
		itemInstance = EnchantmentHelper::enchantItem(random, itemInstance, 30);
	}

	return itemInstance;
}