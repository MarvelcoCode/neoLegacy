#pragma once

#include "../Minecraft.World/WeighedTreasure.h"
#include "../Minecraft.World/ItemInstance.h"
#include "net.minecraft.world.item.h"
#include <unordered_map>
#include <memory>

enum CatchType {
  FISH,
  TREASURE,
  JUNK
};

class CatchTypeWeighedItem : public WeighedRandomItem {
	protected:
		CatchType type;
		int quality;
		int weight;

	public:
		CatchTypeWeighedItem(CatchType type, int quality, int weight) : WeighedRandomItem(weight)
		{
			this->type = type;
			this->quality = quality;
			this->weight = weight;
		}

		CatchType getType()
		{
			return type;
		}

		void calcWeight(int luck) {
			int newWeight = this->weight + this->quality * luck;
			if (newWeight < 0) {
				newWeight = 0;
			}
			this->randomWeight = newWeight;
		}
};

class CatchTypeWeighedItems : public WeighedRandomItemArray {
	public:
		using WeighedRandomItemArray::WeighedRandomItemArray;
		void calcWeights(int luck) {
			for (unsigned int i = 0; i < this->length; i++) {
				CatchTypeWeighedItem* catchTypeWeighedItem = static_cast<CatchTypeWeighedItem *>(this->data[i]);
				catchTypeWeighedItem->calcWeight(luck);
			}
		}
};

class CatchWeighedItem : public WeighedRandomItem {
	protected:
		int itemId;
		int count;
		int auxValue;

	public:
		CatchWeighedItem(int itemId, int count, int auxValue, int weight) : WeighedRandomItem(weight)
		{
			this->itemId = itemId;
			this->count = count;
			this->auxValue = auxValue;
		}
		int getItemId()
		{
			return this->itemId;
		}
		int getCount()
		{
			return this->count;
		}
		int getAuxValue()
		{
			return this->auxValue;
		}
};

class FishingHelper
{
	private:
		FishingHelper();

		CatchTypeWeighedItems catchTypeArray;

		WeighedRandomItemArray fishingFishArray;
		WeighedRandomItemArray fishingJunkArray;
		WeighedRandomItemArray fishingTreasuresArray;
	public:
		// Setup singleton
		FishingHelper(const FishingHelper&) = delete;
		FishingHelper& operator=(const FishingHelper&) = delete;
		static FishingHelper* getInstance();
		std::shared_ptr<ItemInstance> getCatch(int luck, Random* random);
	private:
		CatchType getRandCatchType(int level, Random* random);
		CatchWeighedItem* getRandCatch(CatchType catchType, Random* random);
		std::shared_ptr<ItemInstance> handleCatch(CatchWeighedItem* weighedCatch, CatchType catchType, Random* random);

};