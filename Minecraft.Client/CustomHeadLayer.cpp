#include "stdafx.h"
#include "CustomHeadLayer.h"
#include "LivingEntityRenderer.h"
#include "ModelPart.h"
#include "SkullTileRenderer.h"
#include "TileRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Minecraft.World/ItemInstance.h"
#include "../Minecraft.World/Item.h"
#include "../Minecraft.World/Tile.h"
#include "../Minecraft.World/SkullItem.h"
#include "../Minecraft.World/SkullTileEntity.h"
#include "../Minecraft.World/LivingEntity.h"

CustomHeadLayer::CustomHeadLayer(ModelPart* headPart, LivingEntityRenderer* parentRenderer)
    : headPart(headPart), parentRenderer(parentRenderer)
{
}

int CustomHeadLayer::colorsOnDamage()
{
    return 1;
}

void CustomHeadLayer::render(shared_ptr<LivingEntity> mob,
                              float wp, float ws, float bob,
                              float headRot, float headRotX,
                              float scale, bool useCompiled)
{
    if (mob->instanceof(eTYPE_PLAYER)) return;

    if (!headPart) return;

    shared_ptr<ItemInstance> helmet = mob->getArmor(3);
    if (!helmet) return;

    Item* item = helmet->getItem();
    if (!item) return;

  
    if (dynamic_cast<SkullItem*>(item)) return;

   
    if (item->id >= 256) return;

    
    EntityRenderDispatcher* dispatcher = parentRenderer ? parentRenderer->entityRenderDispatcher : nullptr;

    glPushMatrix();
    headPart->translateTo(1.0f / 16.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    
    if (item->id > 0
        && item->id < Tile::TILE_NUM_COUNT
        && Tile::tiles[item->id] != nullptr
        && TileRenderer::canRender(Tile::tiles[item->id]->getRenderShape()))
    {
        float s = 10.0f / 16.0f;
        glTranslatef(0.0f, -4.0f / 16.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(s, -s, s);

        
        TileRenderer tr;
        tr.renderTile(Tile::tiles[item->id], helmet->getAuxValue(), 1.0f, 1.0f, useCompiled);
        glPopMatrix();
        return;
    }

    
    if (dispatcher && dispatcher->itemInHandRenderer)
    {
        dispatcher->itemInHandRenderer->renderItem(mob, helmet, 0);
    }

    glPopMatrix();
}