/*************************************************************************** 
 *   Copyright (C) 2008 by Andrey Afletdinov                               * 
 *   afletdinov@mail.dc.baikal.ru                                          * 
 *                                                                         * 
 *   This program is free software; you can redistribute it and/or modify  * 
 *   it under the terms of the GNU General Public License as published by  * 
 *   the Free Software Foundation; either version 2 of the License, or     * 
 *   (at your option) any later version.                                   * 
 *                                                                         * 
 *   This program is distributed in the hope that it will be useful,       * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        * 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         * 
 *   GNU General Public License for more details.                          * 
 *                                                                         * 
 *   You should have received a copy of the GNU General Public License     * 
 *   along with this program; if not, write to the                         * 
 *   Free Software Foundation, Inc.,                                       * 
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             * 
 ***************************************************************************/

#include "agg.h"
#include "audio.h"
#include "mp2.h"
#include "world.h"
#include "config.h"
#include "castle.h"
#include "error.h"
#include "monster.h"
#include "heroes.h"
#include "battle.h"
#include "rand.h"
#include "m82.h"
#include "game_focus.h"
#include "game_statuswindow.h"
#include "gamearea.h"
#include "sprite.h"
#include "engine.h"
#include "cursor.h"
#include "tools.h"
#include "ai.h"

#define OBSERVATIONTOWERSCOUTE 10

Maps::TilesAddon *AnimationRemoveObject(const Maps::Tiles & tile)
{
    Maps::TilesAddon *addon = NULL;

    switch(tile.GetObject())
    {
	case MP2::OBJ_FLOTSAM:
	case MP2::OBJ_SHIPWRECKSURVIROR:
	case MP2::OBJ_BOTTLE:
        case MP2::OBJ_TREASURECHEST:
        case MP2::OBJ_ANCIENTLAMP:
	case MP2::OBJ_RESOURCE:	addon = const_cast<Maps::Tiles &>(tile).FindResource(); break;

	case MP2::OBJ_ARTIFACT:	addon = const_cast<Maps::Tiles &>(tile).FindArtifact(); break;
	case MP2::OBJ_CAMPFIRE:	addon = const_cast<Maps::Tiles &>(tile).FindCampFire(); break;
	case MP2::OBJ_MONSTER:  addon = const_cast<Maps::Tiles &>(tile).FindMonster(); break;

	default: break;
    }

    if(NULL == addon) return NULL;

    const Rect & area = GameArea::Get().GetRect();
    const Point pos(tile.GetIndex() % world.w() - area.x, tile.GetIndex() / world.w() - area.y);

    const s16 dstx = BORDERWIDTH + TILEWIDTH * pos.x;
    const s16 dsty = BORDERWIDTH + TILEWIDTH * pos.y;

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();

    const Sprite & sprite = AGG::GetICN(MP2::GetICNObject(addon->object), addon->index);

    Surface sf(tile.GetSurface());
    sf.Blit(sprite, sprite.x(), sprite.y());

    LocalEvent & le = LocalEvent::GetLocalEvent();
    u32 ticket = 0;
    u8 alpha = 250;

    while(le.HandleEvents() && alpha > 0)
    {
	if(!(ticket % ANIMATION_HIGH))
        {
	    cursor.Hide();
	    tile.RedrawTile();
	    tile.RedrawBottom(addon);
            sf.SetAlpha(alpha);
	    display.Blit(sf, dstx, dsty);
	    tile.RedrawTop();
	    if(Game::Focus::HEROES == Game::Focus::Get().Type()) Game::Focus::Get().GetHeroes().Redraw(false);
            cursor.Show();
            display.Flip();
            alpha -= 10;
        }

        ++ticket;
    }
    
    return addon;
}

// action to next cell
void Heroes::Action(const Maps::Tiles & dst)
{
    const u16 & dst_index = dst.GetIndex();
    const MP2::object_t & object = dst.GetObject();

    switch(object)
    {
	case MP2::OBJ_MONSTER:	ActionToMonster(dst_index); break;

        case MP2::OBJ_CASTLE:	ActionToCastle(dst_index); break;
        case MP2::OBJ_HEROES:	ActionToHeroes(dst_index); break;

        case MP2::OBJ_BOAT:	ActionToBoat(dst_index); break;
	case MP2::OBJ_COAST:	ActionToCoast(dst_index); break;

        // resource object
        case MP2::OBJ_MAGICGARDEN:
        case MP2::OBJ_LEANTO:
        case MP2::OBJ_WINDMILL:
        case MP2::OBJ_WATERWHEEL:	ActionToResource(dst_index); break;

        // pickup object
        case MP2::OBJ_RESOURCE:
        case MP2::OBJ_BOTTLE:
        case MP2::OBJ_CAMPFIRE:		ActionToPickupResource(dst_index); break;

        case MP2::OBJ_TREASURECHEST:	ActionToTreasureChest(dst_index); break;
        case MP2::OBJ_ANCIENTLAMP:	ActionToAncientLamp(dst_index); break;
        case MP2::OBJ_FLOTSAM:		ActionToFlotSam(dst_index); break;

        case MP2::OBJ_SHIPWRECKSURVIROR:
        case MP2::OBJ_ARTIFACT: 	ActionToArtifact(dst_index); break;

        // shrine circle
	case MP2::OBJ_SHRINE1:
	case MP2::OBJ_SHRINE2:
        case MP2::OBJ_SHRINE3:		ActionToShrine(dst_index); break;

        // witchs hut
        case MP2::OBJ_WITCHSHUT: 	ActionToWitchsHut(dst_index); break;

        // info message
        case MP2::OBJ_SIGN:		ActionToSign(dst_index); break;

        // luck modification
        case MP2::OBJ_FOUNTAIN:
        case MP2::OBJ_FAERIERING:
        case MP2::OBJ_IDOL:		ActionToLuckObject(dst_index); break;

        case MP2::OBJ_MAGICWELL: 	ActionToMagicWell(dst_index); break;
        case MP2::OBJ_TRADINGPOST:	ActionToTradingPost(dst_index); break;

        // primary skill modification
        case MP2::OBJ_FORT:
        case MP2::OBJ_MERCENARYCAMP:
        case MP2::OBJ_DOCTORHUT:
        case MP2::OBJ_STANDINGSTONES:	ActionToPrimarySkillObject(dst_index); break;

        // morale modification
        case MP2::OBJ_OASIS:
        case MP2::OBJ_TEMPLE:
        case MP2::OBJ_BUOY:		ActionToMoraleObject(dst_index); break;

        // experience modification
        case MP2::OBJ_GAZEBO:		ActionToExperienceObject(dst_index); break;

        // teleports
	case MP2::OBJ_STONELIGHTS:	ActionToTeleports(dst_index); break;

	case MP2::OBJ_OBSERVATIONTOWER:	Maps::ClearFog(Point(dst_index % world.w(), dst_index / world.h()), OBSERVATIONTOWERSCOUTE, GetColor()); break;

	// capture color object
	case MP2::OBJ_ALCHEMYLAB:
        case MP2::OBJ_MINES:
	case MP2::OBJ_SAWMILL:
        case MP2::OBJ_LIGHTHOUSE:
        case MP2::OBJ_DRAGONCITY:
        case MP2::OBJ_ABANDONEDMINE:	ActionToCaptureObject(dst_index); break;

	// accept army
        case MP2::OBJ_WATCHTOWER:
        case MP2::OBJ_EXCAVATION:
        case MP2::OBJ_CAVE:
        case MP2::OBJ_TREEHOUSE:
	case MP2::OBJ_ARCHERHOUSE:
        case MP2::OBJ_GOBLINHUT:
        case MP2::OBJ_DWARFCOTT:
        case MP2::OBJ_HALFLINGHOLE:
	case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT:	ActionToJoinArmy(dst_index); break;

	// recruit army
        case MP2::OBJ_RUINS:
        case MP2::OBJ_TREECITY:
        case MP2::OBJ_WAGONCAMP:
        case MP2::OBJ_TROLLBRIDGE:
	case MP2::OBJ_DESERTTENT:	ActionToRecruitArmy(dst_index); break;

        // object
        case MP2::OBJ_SKELETON:
        case MP2::OBJ_DAEMONCAVE:
        case MP2::OBJ_GRAVEYARD:
        case MP2::OBJ_OBELISK:
	case MP2::OBJ_ORACLE:
	case MP2::OBJ_DERELICTSHIP:
        case MP2::OBJ_TREEKNOWLEDGE:
        case MP2::OBJ_HILLFORT:
        case MP2::OBJ_CRAKEDLAKE:
	case MP2::OBJ_PIRAMID:
        case MP2::OBJ_CITYDEAD:
        case MP2::OBJ_SPHINX:
        case MP2::OBJ_WAGON:
        case MP2::OBJ_ARTESIANSPRING:
        case MP2::OBJ_XANADU:
        case MP2::OBJ_FREEMANFOUNDRY:

        case MP2::OBJ_JAIL:
        case MP2::OBJ_WATERALTAR:
        case MP2::OBJ_AIRALTAR:
        case MP2::OBJ_FIREALTAR:
        case MP2::OBJ_EARTHALTAR:
        case MP2::OBJ_BARROWMOUNDS:
        case MP2::OBJ_ARENA:
        case MP2::OBJ_STABLES:
        case MP2::OBJ_ALCHEMYTOWER:
        case MP2::OBJ_HUTMAGI:
        case MP2::OBJ_EYEMAGI:
        case MP2::OBJ_SIRENS:
        case MP2::OBJ_MERMAID:
	    if(H2Config::Debug()) Error::Verbose("Heroes::Action: FIXME: " + std::string(MP2::StringObject(object)));
	    break;

	default: break;
    }
}

void Heroes::ActionToMonster(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const Monster::monster_t monster = Monster::Monster(tile);
    const u16 count = Monster::GetSize(tile);
    std::vector<Army::Troops> army;
    int tr_c = count > 5 ? 5 : count;

    for(int i=0; i< tr_c; i++)
    {
	Army::Troops troop(monster, (int)(count/tr_c));
	army.push_back(troop);
    }

    std::string str = "Heroes::ActionToMonster: " + GetName() + " attack monster " + Monster::String(monster)+" (";
    String::AddInt(str, count);
    str += ")";
    if(H2Config::Debug()) Error::Verbose(str);

    Display::Fade();

    switch(Army::Battle(*this, army, tile))
    {
	case Army::WIN:
	{
	    Maps::TilesAddon *addon = tile.FindMonster();
	    if(addon)
	    {
		AGG::PlaySound(M82::KILLFADE);
		const u32 uniq = addon->uniq;
		AnimationRemoveObject(tile);
		tile.Remove(uniq);
		tile.SetObject(MP2::OBJ_ZERO);

		// remove shadow from left cell
		if(Maps::isValidDirection(dst_index, Direction::LEFT))
		    world.GetTiles(Maps::GetDirectionIndex(dst_index, Direction::LEFT)).Remove(uniq);
	    }
	    IncreaseExperience(1000);	//TODO: Figure out appropriate calculation
	}
	break;

	case Army::RETREAT:
	case Army::SURRENDER:
	case Army::LOSE:
	    AGG::PlaySound(M82::KILLFADE);
    	    Maps::Tiles & tile_hero = world.GetTiles(GetCenter());
    	    tile_hero.SetObject(GetUnderObject());

    	    world.GetKingdom(color).RemoveHeroes(this);
    	    SetFreeman();
	break;
    }

    // redraw focus list
    Game::Focus::Get().Redraw();

    // redraw status
    Game::StatusWindow::Get().SetState(Game::StatusWindow::DAY);
    Game::StatusWindow::Get().Redraw();
}

void Heroes::ActionToHeroes(const u16 dst_index)
{
    const Heroes *other_hero = world.GetHeroes(dst_index);

    if(! other_hero) return;

    if(color == other_hero->GetColor())
    {
	if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: " + GetName() + " meeting " + other_hero->GetName());

	MeetingDialog(const_cast<Heroes &>(*other_hero));
    }
    else
    {
	Army::battle_t b = Army::Battle(*this, const_cast<Heroes &>(*other_hero), world.GetTiles(dst_index));

	if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: " + GetName() + " attack enemy hero " + other_hero->GetName());

	switch(b)
	{
	    case Army::WIN: if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: result WIN"); break;
	    case Army::LOSE: if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: result LOSE"); break;
	    case Army::RETREAT: if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: result RETREAT"); break;
	    case Army::SURRENDER: if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: result SURRENDER"); break;
	}

	if(H2Config::Debug()) Error::Verbose("Heroes::ActionToHeroes: FIXME: attack enemy hero");
    }
}

void Heroes::ActionToCastle(const u16 dst_index)
{
    const Castle *castle = world.GetCastle(dst_index);

    if(! castle) return;

    if(color == castle->GetColor())
    {
	if(H2Config::Debug()) Error::Verbose("Heroes::ActionToCastle: " + GetName() + " goto castle " + castle->GetName());

	Mixer::Reduce();

	const_cast<Castle *>(castle)->OpenDialog();

	Mixer::Enhance();
    }
    else
    {
	if(H2Config::Debug()) Error::Verbose("Heroes::ActionToCastle: " + GetName() + " attack enemy castle " + castle->GetName());

	if(H2Config::Debug()) Error::Verbose("Heroes::ActiontoCastle: FIXME: attack enemy castle");
    }
}

void Heroes::ActionToBoat(const u16 dst_index)
{
    if(isShipMaster()) return;

    const u16 from_index = Maps::GetIndexFromAbsPoint(mp);

    Maps::Tiles & tiles_from = world.GetTiles(from_index);
    Maps::Tiles & tiles_to = world.GetTiles(dst_index);

    move_point = 0;

    tiles_from.SetObject(MP2::OBJ_COAST);

    SetCenter(dst_index);
    SetShipMaster(true);

    tiles_to.SetObject(MP2::OBJ_HEROES);

    save_maps_general = MP2::OBJ_ZERO;
    if(H2Config::MyColor() == GetColor()) AGG::PlaySound(M82::KILLFADE);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToBoat: " + GetName() + " to boat");
}

void Heroes::ActionToCoast(const u16 dst_index)
{
    if(! isShipMaster()) return;

    const u16 from_index = Maps::GetIndexFromAbsPoint(mp);

    Maps::Tiles & tiles_from = world.GetTiles(from_index);
    Maps::Tiles & tiles_to = world.GetTiles(dst_index);

    move_point = 0;

    tiles_from.SetObject(MP2::OBJ_BOAT);

    SetCenter(dst_index);
    SetShipMaster(false);

    tiles_to.SetObject(MP2::OBJ_HEROES);

    save_maps_general = MP2::OBJ_COAST;
    if(H2Config::MyColor() == GetColor()) AGG::PlaySound(M82::KILLFADE);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToCoast: " + GetName() + " to coast");
}

void Heroes::ActionToPickupResource(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::funds_t resource;

    const u8 count = tile.GetQuantity2();
    switch(tile.GetQuantity1())
    {
	case Resource::WOOD: resource.wood += count; break;
    	case Resource::MERCURY: resource.mercury += count; break;
    	case Resource::ORE: resource.ore += count; break;
    	case Resource::SULFUR: resource.sulfur += count; break;
    	case Resource::CRYSTAL: resource.crystal += count; break;
    	case Resource::GEMS: resource.gems += count; break;
    	case Resource::GOLD: resource.gold += 100 * count; break;

	default: break;
    }

    PlayPickupSound();
    AnimationRemoveObject(tile);

    world.GetKingdom(GetColor()).AddFundsResource(resource);

    // dialog
    if(H2Config::MyColor() == GetColor())
	switch(tile.GetObject())
	{
	    case MP2::OBJ_CAMPFIRE:
		Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), "Ransacking an enemy camp, you discover a hidden cache of treasures.", resource);
	    break;

	    case MP2::OBJ_BOTTLE:
		Dialog::Message(MP2::StringObject(tile.GetObject()), world.MessageSign(dst_index), Font::BIG, Dialog::OK);
	    break;

	    default: break;
	}

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    // redraw status info
    Game::StatusWindow::Get().Redraw();

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToPickupResource: " + GetName() + " pickup small resource");
}

void Heroes::ActionToResource(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::funds_t resource;

    const u8 count = tile.GetQuantity2();
    switch(tile.GetQuantity1())
    {
	case Resource::WOOD: resource.wood += count; break;
    	case Resource::MERCURY: resource.mercury += count; break;
    	case Resource::ORE: resource.ore += count; break;
    	case Resource::SULFUR: resource.sulfur += count; break;
    	case Resource::CRYSTAL: resource.crystal += count; break;
    	case Resource::GEMS: resource.gems += count; break;
    	case Resource::GOLD: resource.gold += 100 * count; break;

	default: break;
    }

    PlayPickupSound();
    AnimationRemoveObject(tile);

    world.GetKingdom(GetColor()).AddFundsResource(resource);

    // dialog
    if(H2Config::MyColor() == GetColor())
	switch(tile.GetObject())
	{
	    case MP2::OBJ_WINDMILL:
	    	if(resource.GetValidItems())
		    Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), "The keeper of the mill announces: \"Milord, I have been working very hard to provide you with these resources, come back next week for more.\"", resource);
		else
		    Dialog::Message(MP2::StringObject(tile.GetObject()), "The keeper of the mill announces: \"Milord, I am sorry, there are no resources currently available. Please try again next week.\"", Font::BIG, Dialog::OK);
	    break;

	    case MP2::OBJ_WATERWHEEL:
	    	if(resource.GetValidItems())
		    Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), "The keeper of the mill announces: \"Milord, I have been working very hard to provide you with this gold, come back next week for more.\"", resource);
		else
		    Dialog::Message(MP2::StringObject(tile.GetObject()), "The keeper of the mill announces: \"Milord, I am sorry, there is no gold currently available.  Please try again next week.\"", Font::BIG, Dialog::OK);
	    break;
	    
	    case MP2::OBJ_LEANTO:
	    	if(resource.GetValidItems())
		    Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), "You've found an abandoned lean-to. Poking about, you discover some resources hidden nearby.", resource);
		else
		    Dialog::Message(MP2::StringObject(tile.GetObject()), "The lean-to is long abandoned. There is nothing of value here.", Font::BIG, Dialog::OK);
	    break;

    	    case MP2::OBJ_MAGICGARDEN:
	    	if(resource.GetValidItems())
		    Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), "You catch a leprechaun foolishly sleeping amidst a cluster of magic mushrooms. In exchange for his freedom, he guides you to a smallpot filled with precious things.", resource);
		else
		    Dialog::Message(MP2::StringObject(tile.GetObject()), "You've found a magic garden, the kind of place that leprechauns and faeries like to cavort in, but there is no one here today. Perhaps you should try again next week.", Font::BIG, Dialog::OK);
    	    break;

	    default: break;
	}

    tile.SetQuantity1(0);
    tile.SetQuantity2(0);

    // redraw status info
    Game::StatusWindow::Get().Redraw();

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToResource: " + GetName() + " pickup small resource");
}

void Heroes::ActionToFlotSam(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    std::string body;
    Resource::funds_t resource;

    resource.gold += 100 * tile.GetQuantity1();
    resource.wood += tile.GetQuantity2();

    if(resource.gold && resource.wood)
	body = "You search through the flotsam, and find some wood and some gold.";
    else
    if(resource.wood)
	body = "You search through the flotsam, and find some wood.";
    else
	body = "You search through the flotsam, but find nothing.";

    PlayPickupSound();
    AnimationRemoveObject(tile);

    world.GetKingdom(GetColor()).AddFundsResource(resource);
    if(H2Config::MyColor() == GetColor())
    {
	if(resource.GetValidItems())
	    Dialog::ResourceInfo(MP2::StringObject(tile.GetObject()), body, resource);
	else
	    Dialog::Message(MP2::StringObject(tile.GetObject()), body, Font::BIG, Dialog::OK);
    }

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    // redraw status info
    Game::StatusWindow::Get().Redraw();

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToFlotSam: " + GetName() + " pickup small resource");
}

void Heroes::ActionToShrine(const u16 dst_index)
{
    const Spell::spell_t spell = world.SpellFromShrine(dst_index);

    const std::string & spell_name = Spell::String(spell);
    const u8 spell_level = Spell::Level(spell);

    std::string head;
    std::string body("You come across");

    switch(spell_level)
    {
	case 1:
	    head = "Shrine of the 1st Circle";
	    body += " a small shrine attended by a group of novice acolytes. In exchange for your protection, they agree to teach you a simple spell -";
	    break;
	case 2:
	    head = "Shrine of the 2st Circle";
	    body += " an ornate shrine attended by a group of rotund friars. In exchange for your protection, they agree to teach you a spell -";
	    break;
	case 3:
	    head = "Shrine of the 3st Circle";
	    body += " a lavish shrine attended by a group of high priests. In exchange for your protection, they agree to teach you a sophisticated spell -";
	    break;
	default: return;
    }
    
    body += "'" + spell_name + "'.";

    // check spell book
    if(!HasArtifact(Artifact::MAGIC_BOOK))
    {
	body += " Unfortunately, you have no Magic Book to record the spell with.";
	if(H2Config::MyColor() == GetColor()) Dialog::Message(head, body, Font::BIG, Dialog::OK);
	return;
    }

    // check valid level spell and wisdom skill
    if(3 == spell_level && Skill::Level::NONE == GetLevelSkill(Skill::Secondary::WISDOM))
    {
	body += " Unfortunately, you do not have the wisdom to understand the spell, and you are unable to learn it.";
	if(H2Config::MyColor() == GetColor()) Dialog::Message(head, body, Font::BIG, Dialog::OK);
	return;
    }

    AppendSpellToBook(spell);
    SetVisited(dst_index, Visit::GLOBAL);

    if(H2Config::MyColor() == GetColor())
    {
	AGG::PlaySound(M82::TREASURE);
	Dialog::SpellInfo(spell_name, body, spell);
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToShrine: " + GetName());
}

void Heroes::ActionToWitchsHut(const u16 dst_index)
{
    const Skill::Secondary::skill_t skill = world.SkillFromWitchsHut(dst_index);
    const std::string & skill_name = Skill::Secondary::String(skill);
    const std::string head("Witch's Hut");

    // check full
    if(HEROESMAXSKILL == secondary_skills.size())
    {
	const std::string body("You approach the hut and observe a witch inside studying an ancient tome on " + skill_name + ". As you approach, she turns and focuses her one glass eye on you. \"You already know everything you deserve to learn!\" the witch screeches. \"NOW GET OUT OF MY HOUSE!\"");
	if(H2Config::MyColor() == GetColor()) Dialog::Message(head, body, Font::BIG, Dialog::OK);
	return;
    }

    // check present skill
    if(HasSecondarySkill(skill))
    {
	const std::string body("You approach the hut and observe a witch inside studying an ancient tome on " + skill_name + ". As you approach, she turns and speaks. \"You already know that which I would teach you. I can help you no further.\"");
	if(H2Config::MyColor() == GetColor()) Dialog::Message(head, body, Font::BIG, Dialog::OK);
	return;
    }

    LearnBasicSkill(skill);
    SetVisited(dst_index, Visit::GLOBAL);

    if(H2Config::MyColor() == GetColor())
    {
	const std::string body("An ancient and immortal witch living in a hut with bird's legs for stilts teaches you " + skill_name + " for her own inscrutable purposes.");
	Dialog::SkillInfo(skill_name, body, skill, Skill::Level::BASIC);
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToWitchsHut: " + GetName());
}

void Heroes::ActionToLuckObject(const u16 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    const char *body_true = NULL;
    const char *body_false = NULL;

    switch(obj)
    {
        case MP2::OBJ_FOUNTAIN:
    	    body_false = "You drink from the enchanted fountain, but nothing happens.";
    	    body_true = "As you drink the sweet water, you gain luck for your next battle.";
    	    break;

        case MP2::OBJ_FAERIERING:
    	    body_false = "You enter the faerie ring, but nothing happens.";
    	    body_true = "Upon entering the mystical faerie ring, your army gains luck for its next battle.";
    	    break;

        case MP2::OBJ_IDOL:
	    body_false = "You've found an ancient and weathered stone idol. It is supposed to grant luck to visitors, but since the stars are already smiling upon you, it does nothing.";
	    body_true = "You've found an ancient and weathered stone idol. Kissing it is supposed to be lucky, so you do. The stone is very cold to the touch.";
    	    break;

    	default: return;
    }

    const std::string header(MP2::StringObject(obj));

    // check already visited
    if(isVisited(obj))
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, body_false, Font::BIG, Dialog::OK);
	return;
    }

    // modify luck
    SetVisited(dst_index);

    if(H2Config::MyColor() == GetColor())
    {
	AGG::PlaySound(M82::GOODLUCK);
	Dialog::SpriteInfo(header, body_true, AGG::GetICN(ICN::EXPMRL, 0));
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToLuckObject: " + GetName());
}

void Heroes::ActionToSign(const u16 dst_index)
{
    if(H2Config::MyColor() == GetColor())
	Dialog::Message("Sign", world.MessageSign(dst_index), Font::BIG, Dialog::OK);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToSign: " + GetName());
}

void Heroes::ActionToMagicWell(const u16 dst_index)
{
    const std::string header(MP2::StringObject(MP2::OBJ_MAGICWELL));
    const u16 max_point = GetMaxSpellPoints();

    if(magic_point == max_point)
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, "A drink at the well is supposed to restore your spell points, but you are already at maximum.", Font::BIG, Dialog::OK);
	return;
    }

    // check already visited
    if(isVisited(MP2::OBJ_MAGICWELL))
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, "A second drink at the well in one day will not help you.", Font::BIG, Dialog::OK);
	return;
    }

    SetVisited(dst_index);
    magic_point = max_point;

    if(H2Config::MyColor() == GetColor()) Dialog::Message(header, "A drink from the well has restored your spell points to maximum.", Font::BIG, Dialog::OK);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToMagicWell: " + GetName());
}

void Heroes::ActionToTradingPost(const u16 dst_index)
{
    if(H2Config::MyColor() == GetColor()) Dialog::Marketplace();

    // redraw status info
    Game::StatusWindow::Get().Redraw();

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToTradingPost: " + GetName());
}

void Heroes::ActionToPrimarySkillObject(const u16 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    const char *body_true = NULL;
    const char *body_false = NULL;
    
    Skill::Primary::skill_t skill = Skill::Primary::ATTACK;

    switch(obj)
    {
        case MP2::OBJ_FORT:
    	    skill = Skill::Primary::DEFENCE;
    	    body_false = "\"I'm sorry sir,\" The leader of the soldiers says, \"but you already know everything we have to teach.\"";
    	    body_true = "The soldiers living in the fort teach you a few new defensive tricks.";
    	    break;

        case MP2::OBJ_MERCENARYCAMP:
    	    skill = Skill::Primary::ATTACK;
    	    body_false = "You've come upon a mercenary camp practicing their tactics. \"You're too advanced for us,\" the mercenary captain says. \"We can teach nothing more.\"";
    	    body_true = "You've come upon a mercenary camp practicing their tactics. The mercenaries welcome you and your troops and invite you to train with them.";
    	    break;

        case MP2::OBJ_DOCTORHUT:
    	    skill = Skill::Primary::KNOWLEDGE;
    	    body_false = "\"Go 'way!\", the witch doctor barks, \"you know all I know.\"";
    	    body_true = "An Orcish witch doctor living in the hut deepens your knowledge of magic by showing you how to cast stones, read portents, and decipher the intricacies of chicken entrails.";
    	    break;

        case MP2::OBJ_STANDINGSTONES:
    	    skill = Skill::Primary::POWER;
    	    body_false = "You've found a group of Druids worshipping at one of their strange stone edifices. Silently, the Druids turn you away, indicating they have nothing new to teach you.";
    	    body_true = "You've found a group of Druids worshipping at one of their strange stone edifices. Silently, they teach you new ways to cast spells.";
    	    break;

    	default: return;
    }

    const std::string header(MP2::StringObject(obj));

    // check already visited
    if(isVisited(world.GetTiles(dst_index)))
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, body_false, Font::BIG, Dialog::OK);
	return;
    }

    // increase skill
    SetVisited(dst_index);

    switch(skill)
    {
        case Skill::Primary::DEFENCE:		++defence; break;
        case Skill::Primary::ATTACK:		++attack; break;
        case Skill::Primary::KNOWLEDGE:		++knowledge; break;
        case Skill::Primary::POWER:		++power; break;
        default: break;
    }

    if(H2Config::MyColor() == GetColor()) Dialog::SkillInfo(header, body_true, skill);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToPrimarySkillObject: " + GetName());
}

void Heroes::ActionToMoraleObject(const u16 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    const char *body_true = NULL;
    const char *body_false = NULL;

    switch(obj)
    {
        case MP2::OBJ_BUOY:
    	    body_false = "Your men spot a navigational buoy, confirming that you are on course.";
    	    body_true = "Your men spot a navigational buoy, confirming that you are on course and increasing their morale.";
    	    break;

        case MP2::OBJ_OASIS:
    	    body_false = "The drink at the oasis is refreshing, but offers no further benefit. The oasis might help again if you fought a battle first.";
    	    body_true = "A drink at the oasis fills your troops with strength and lifts their spirits.  You can travel a bit further today.";
    	    break;

        case MP2::OBJ_TEMPLE:
    	    body_false = "It doesn't help to pray twice before a battle. Come back after you've fought.";
    	    body_true = "A visit and a prayer at the temple raises the morale of your troops.";
    	    break;

/*
	FIXME bud morale modification
        case MP2::OBJ_GRAVEYARD:
    	    body_false = "";
    	    body_true = "";
    	    break;

        case MP2::OBJ_SHIPWRECK:
    	    body_false = "";
    	    body_true = "";
    	    break;

        case MP2::OBJ_DERELICTSHIP:
    	    body_false = "";
    	    body_true = "";
    	    break;

	{Graveyard}
	You tentatively approach the burial ground of ancient warriors.  Do you want to search the graves?
	Upon defeating the Zombies you spend several hours searching the graves and find nothing.  Such a despicable act reduces your army's morale.
	Upon defeating the zomies you search the graves and find something!
	{Shipwreck}
	The rotting hulk of a great pirate ship creaks eerily as it is pushed against the rocks.  Do you wish to search the shipwreck?
	Upon defeating the Ghosts you spend several hours sifting through the debris and find nothing.  Such a despicable act reduces your army's morale.
	Upon defeating the Ghosts you sift through the debris and find something!
	{Derelict Ship}
	The rotting hulk of a great pirate ship creaks eerily as it is pushed against the rocks.  Do you wish to search the ship?
	Upon defeating the Skeletons you spend several hours sifting through the debris and find nothing.  Such a despicable act reduces your army's morale.
	Upon defeating the Skeletons you sift through the debris and find something!
*/
    	default: return;
    }

    const std::string header(MP2::StringObject(obj));

    // check already visited
    if(isVisited(obj))
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, body_false, Font::BIG, Dialog::OK);
	return;
    }

    // modify morale
    SetVisited(dst_index);

    if(H2Config::MyColor() == GetColor())
    {
	AGG::PlaySound(M82::GOODMRLE);
	Dialog::SpriteInfo(header, body_true, AGG::GetICN(ICN::EXPMRL, 2));
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToMoraleObject: " + GetName());
}

void Heroes::ActionToExperienceObject(const u16 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    const char *body_true = NULL;
    const char *body_false = NULL;
    
    u16 exp = 0;

    switch(obj)
    {
        case MP2::OBJ_GAZEBO:
    	    body_false = "An old Knight appears on the steps of the gazebo. \"I am sorry, my liege, I have taught you all I can.\"";
    	    body_true = "An old Knight appears on the steps of the gazebo. \"My liege, I will teach you all that I know to aid you in your travels.\"";
    	    exp = 1000;
    	    break;

    	default: return;
    }

    const std::string header(MP2::StringObject(obj));

    // check already visited
    if(isVisited(world.GetTiles(dst_index)))
    {
	if(H2Config::MyColor() == GetColor()) Dialog::Message(header, body_false, Font::BIG, Dialog::OK);
	return;
    }

    // visit
    SetVisited(dst_index);

    AGG::PlaySound(M82::EXPERNCE);
    if(H2Config::MyColor() == GetColor())
    {
	std::string count;
	String::AddInt(count, exp);
	const Sprite & sprite = AGG::GetICN(ICN::EXPMRL, 4);
	Surface image(sprite.w(), sprite.h() + 12);
	image.SetColorKey();
	image.Blit(sprite);
	Text text(count, Font::SMALL);
	text.Blit((sprite.w() - Text::width(count, Font::SMALL)) / 2, sprite.h() + 2, image);
	Dialog::SpriteInfo(header, body_true, image);
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToExperienceObject: " + GetName());

    IncreaseExperience(exp);
}

void Heroes::ActionToArtifact(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    const Artifact::artifact_t art = Artifact::Artifact(tile.GetQuantity1());
    const Sprite & border = AGG::GetICN(ICN::RESOURCE, 7);
    Surface sprite(border.w(), border.h());

    sprite.Blit(border);
    sprite.Blit(AGG::GetICN(ICN::ARTIFACT, art), 5, 5);

    switch(tile.GetObject())
    {
        case MP2::OBJ_SHIPWRECKSURVIROR:
	    if(H2Config::MyColor() == GetColor()) Dialog::SpriteInfo(MP2::StringObject(tile.GetObject()), "You've pulled a shipwreck survivor from certain death in an unforgiving ocean.  Grateful, he rewards you for your act of kindness by giving you the " + Artifact::String(art), sprite);
        break;

	case MP2::OBJ_ARTIFACT:
/* FIXME: pickup artifact variants
    {Artifact}
    You come upon an ancient artifact.  As you reach for it, a pack of Rogues leap out of the brush to guard their stolen loot.
    {Artifact}
    Through a clearing you observe an ancient artifact.  Unfortunately, it's guarded by a nearby %s.  Do you want to fight the %s for the artifact?
    Victorious, you take your prize, the %s
    Discretion is the better part of valor, and you decide to avoid this fight for today.
    {Artifact}
    You've found the humble dwelling of a withered hermit.  The hermit tells you that he is willing to give the %s to the first wise person he meets.
    {Artifact}
    You've come across the spartan quarters of a retired soldier.  The soldier tells you that he is willing to pass on the %s to the first true leader he meets.
    {Artifact}
    A leprechaun offers you the %s for the small price of 2000 gold.  Do you wish to buy this artifact?
    You try to pay the leprechaun, but realize that you can't afford it.  The leprechaun stamps his foot and ignores you.
    Insulted by your refusal of his generous offer, the leprechaun stamps his foot and ignores you.
    {Artifact}
    A leprechaun offers you the %s for the small price of 2500 gold and 3 %s.  Do you wish to buy this artifact?
    You try to pay the leprechaun, but realize that you can't afford it.  The leprechaun stamps his foot and ignores you.
    Insulted by your refusal of his generous offer, the leprechaun stamps his foot and ignores you.
    {Artifact}
    A leprechaun offers you the %s for the small price of 3000 gold and 5 %s.  Do you wish to buy this artifact?
    You try to pay the leprechaun, but realize that you can't afford it.  The leprechaun stamps his foot and ignores you.
    Insulted by your refusal of his generous offer, the leprechaun stamps his foot and ignores you.
*/
	    if(H2Config::MyColor() == GetColor()) Dialog::SpriteInfo(MP2::StringObject(tile.GetObject()), "You've found the artifact: " + Artifact::String(art), sprite);
	break;

	default: break;
    }
    PlayPickupSound();
    AnimationRemoveObject(tile);

    PickupArtifact(art);

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToArtifact: " + GetName() + " pickup artifact");
}

void Heroes::ActionToTreasureChest(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    PlayPickupSound();
    AnimationRemoveObject(tile);

    Resource::funds_t resource;
    resource.gold = tile.GetQuantity2() * 100;

    // dialog
    if(Maps::Ground::WATER == tile.GetGround())
    {
	std::string message("After spending hours trying to fish the chest out of the sea,");

	if(0 == resource.gold)
	{
	    message += " you open it, only to find it empty.";
	    if(H2Config::MyColor() == GetColor())  Dialog::Message("Chest", message, Font::BIG, Dialog::OK);
	}
	else
	if(tile.GetQuantity1())
	{
	    const Artifact::artifact_t art = Artifact::Artifact(tile.GetQuantity1());
	    if(H2Config::MyColor() == GetColor())
	    {
		    std::string count;
		    String::AddInt(count, resource.gold);
		    message += " you open it and find " + count + " gold and the " + Artifact::String(art);
		    const Sprite & gold = AGG::GetICN(ICN::RESOURCE, 6);
		    const Sprite & border = AGG::GetICN(ICN::RESOURCE, 7);
		    const Sprite & artifact = AGG::GetICN(ICN::ARTIFACT, art + 1);
		    Surface image(gold.w() + border.w() + 50, border.h());
		    image.SetColorKey();
		    image.Blit(border);
		    image.Blit(artifact, 5, 5);
		    image.Blit(gold, border.w() + 50, (border.h() - gold.h()) / 2);
		    Text text(count, Font::SMALL);
		    text.Blit(border.w() + 50 + (gold.w() - Text::width(count, Font::SMALL)) / 2, border.h() - 25, image);
		    Dialog::SpriteInfo("Chest", message, image);
	    }
	    PickupArtifact(art);
	    world.GetKingdom(GetColor()).AddFundsResource(resource);
	}
	else
	{
	    if(H2Config::MyColor() == GetColor())
	    {
		    std::string count;
		    String::AddInt(count, resource.gold);
		    message += " you open it and find " + count + " gold pieces.";
		    const Sprite & gold = AGG::GetICN(ICN::RESOURCE, 6);
		    Surface image(gold.w(), gold.h() + 12);
		    image.SetColorKey();
		    image.Blit(gold);
		    Text text(count, Font::SMALL);
		    text.Blit((gold.w() - Text::width(count, Font::SMALL)) / 2, gold.h(), image);
		    Dialog::SpriteInfo("Chest", message, image);
	    }
	    world.GetKingdom(GetColor()).AddFundsResource(resource);
	}
    }
    else
    {
	std::string message("After scouring the area,");

	if(tile.GetQuantity1())
	{
	    const Artifact::artifact_t art = Artifact::Artifact(tile.GetQuantity1());
	    if(H2Config::MyColor() == GetColor())
	    {
		    message += " you fall upon a hidden chest, containing the ancient artifact " + Artifact::String(art);
		    const Sprite & border = AGG::GetICN(ICN::RESOURCE, 7);
		    const Sprite & artifact = AGG::GetICN(ICN::ARTIFACT, art + 1);
		    Surface image(border.w(), border.h());
		    image.SetColorKey();
		    image.Blit(border);
		    image.Blit(artifact, 5, 5);
		    Dialog::SpriteInfo("Chest", message, image);
	    }
	    PickupArtifact(art);
	    world.GetKingdom(GetColor()).AddFundsResource(resource);
	}
	else
	{
	    const u16 expr = resource.gold - 500;
	    message += " you fall upon a hidden treasure cache. You may take the gold or distribute the gold to the peasants for experience. Do you wish to keep the gold?";

	    if((H2Config::MyColor() == GetColor() && Dialog::SelectGoldOrExp("Chest", message, resource.gold, expr)) ||
		(H2Config::MyColor() != GetColor() && AI::SelectGoldOrExp(*this, resource.gold, expr)))
		    world.GetKingdom(GetColor()).AddFundsResource(resource);
	    else
		    IncreaseExperience(expr);
	}
    }

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    // redraw status info
    Game::StatusWindow::Get().Redraw();

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToTreasureChest: " + GetName() + " pickup chest");
}

void Heroes::ActionToAncientLamp(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    const u32 count = Rand::Get(2, 4);
    const u32 army_size = GetCountArmy();

    if(((HEROESMAXARMY == army_size && HasMonster(Monster::GENIE)) || HEROESMAXARMY > army_size))
    {
	const std::string message("You stumble upon a dented and tarnished lamp lodged deep in the earth. Do you wish to rub the lamp?");
	if(H2Config::MyColor() != GetColor())
	{
	    AI::RecruitTroops(*this, Monster::GENIE, count);

	    tile.RemoveObjectSprite();
	    tile.SetObject(MP2::OBJ_ZERO);
	}
	else
	// recruit
	if(Dialog::YES == Dialog::Message(Monster::String(Monster::GENIE), message, Font::BIG, Dialog::YES|Dialog::NO))
	{
	    PlayPickupSound();
	    AnimationRemoveObject(tile);

	    const u16 recruit = Dialog::RecruitMonster(Monster::GENIE, count);
	    if(recruit) JoinTroops(Monster::GENIE, recruit);

	    // post tile action
	    tile.RemoveObjectSprite();
	    tile.SetObject(MP2::OBJ_ZERO);

	    // redraw status info
	    Game::StatusWindow::Get().Redraw();
	}
    }
    // is full
    else
	Dialog::Message(Monster::String(Monster::GENIE), "You are unable to recruit at this time, your ranks are full.", Font::BIG, Dialog::OK);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToTreasureChest: " + GetName() + " pickup chest");
}

void Heroes::ActionToTeleports(const u16 index_from)
{
    const u16 index_to = world.NextTeleport(index_from);

    if(index_from == index_to)
    {
	Error::Warning("Heroes::ActionToTeleports: action unsuccessfully...");
	return;
    }

    Maps::Tiles & tiles_from = world.GetTiles(index_from);
    Maps::Tiles & tiles_to = world.GetTiles(index_to);

    if(MP2::OBJ_HEROES != save_maps_general)
    {
	tiles_from.SetObject(MP2::OBJ_STONELIGHTS);
    }

    SetCenter(index_to);

    save_maps_general = MP2::OBJ_STONELIGHTS;
    tiles_to.SetObject(MP2::OBJ_HEROES);
}

/* capture color object */
void Heroes::ActionToCaptureObject(const u16 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    std::string header;
    std::string body;

    Resource::resource_t res = Resource::UNKNOWN;
    const Sprite *sprite = NULL;

    switch(obj)
    {
	case MP2::OBJ_ALCHEMYLAB:
	    sprite = &AGG::GetICN(ICN::RESOURCE, 1);
	    res = Resource::MERCURY;
	    header = MP2::StringObject(obj);
	    body = "You have taken control of the local Alchemist shop. It will provide you with one unit of Mercury per day.";
	    break;
        case MP2::OBJ_MINES:
    	{
    	    const Maps::TilesAddon * taddon = world.GetTiles(dst_index).FindMines();

            // ore
            if(0 == taddon->index)
            {
		sprite = &AGG::GetICN(ICN::RESOURCE, 2);
        	res = Resource::ORE;
        	header = "Ore Mine";
        	body = "You gain control of an ore mine. It will provide you with two units of ore per day.";
            }
            else
            // sulfur
            if(1 == taddon->index)
            {
		sprite = &AGG::GetICN(ICN::RESOURCE, 3);
        	res = Resource::SULFUR;
        	header = "Sulfur Mine";
		body = "You gain control of a sulfur mine. It will provide you with one unit of sulfur per day.";
            }
            else
            // crystal
            if(2 == taddon->index)
            {
		sprite = &AGG::GetICN(ICN::RESOURCE, 4);
        	res = Resource::CRYSTAL;
        	header = "Crystal Mine";
		body = "You gain control of a crystal mine. It will provide you with one unit of crystal per day.";
            }
            else
            // gems
            if(3 == taddon->index)
            {
		sprite = &AGG::GetICN(ICN::RESOURCE, 5);
        	res = Resource::GEMS;
        	header = "Gems Mine";
		body = "You gain control of a gem mine. It will provide you with one unit of gems per day.";
            }
            else
            // gold
            if(4 == taddon->index)
            {
		sprite = &AGG::GetICN(ICN::RESOURCE, 6);
        	res = Resource::GOLD;
        	header = "Gold Mine";
		body = "You gain control of a gold mine. It will provide you with 1000 gold per day.";
            }
    	}
    	    break;
	case MP2::OBJ_SAWMILL:
	    sprite = &AGG::GetICN(ICN::RESOURCE, 0);
    	    res = Resource::WOOD;
	    header = MP2::StringObject(obj);
	    body = "You gain control of a sawmill. It will provide you with two units of wood per day.";
	    break;

        case MP2::OBJ_LIGHTHOUSE:
	    header = MP2::StringObject(obj);
    	    body = "The lighthouse is now under your control, and all of your ships will now move further each turn.";
	    break;

	case MP2::OBJ_ABANDONEDMINE:
    	    Error::Warning("Heroes::ActionToCaptureObject: FIXME: Abandone Mine");
	    break;

	case MP2::OBJ_DRAGONCITY:
    	    Error::Warning("Heroes::ActionToCaptureObject: FIXME: Dragon City");
    	    // message variant:
	    //The Dragon city has no Dragons willing to join you this week.  Perhaps a Dragon will become available next week.
	    //You stand before the Dragon City, a place off-limits to mere humans.  Do you wish to violate this rule and challenge the Dragons to a fight?
	    //Having defeated the Dragon champions, the city's leaders agree to supply some Dragons to your army for a price.  Do you wish to recruit Dragons?
	    //The Dragon city is willing to offer some Dragons for your army for a price.  Do you wish to recruit Dragons?
	    break;

        default:
    	    Error::Warning("Heroes::ActionToCaptureObject: unknown captured: " + std::string(MP2::StringObject(obj)));
    	    return;
    }

    // capture object
    if(GetColor() != world.ColorCapturedObject(dst_index))
    {
	world.CaptureObject(dst_index, GetColor());
	world.GetTiles(dst_index).CaptureFlags32(obj, GetColor());
	if(H2Config::MyColor() == GetColor() && sprite) Dialog::SpriteInfo(header, body, *sprite);
    }

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToCaptureObject: " + GetName() + " captured: " + std::string(MP2::StringObject(obj)));
}

void Heroes::ActionToJoinArmy(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();

    switch(obj)
    {
        case MP2::OBJ_WATCHTOWER:
        case MP2::OBJ_EXCAVATION:
        case MP2::OBJ_CAVE:
        case MP2::OBJ_TREEHOUSE:
	case MP2::OBJ_ARCHERHOUSE:
        case MP2::OBJ_GOBLINHUT:
        case MP2::OBJ_DWARFCOTT:
        case MP2::OBJ_HALFLINGHOLE:
	case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT: break;
	default: return;
    }
    const Monster::monster_t monster = Monster::Monster(obj);
    const u32 count = tile.GetQuantity2() * 0xFF + tile.GetQuantity1();
    const u32 army_size = GetCountArmy();
    if(count)
    {
	if(((HEROESMAXARMY == army_size && HasMonster(monster)) ||
		HEROESMAXARMY > army_size))
	{
	    // join
	    const std::string & message = "A group of " + Monster::String(monster) + " with a desire for greater glory wish to join you. Do you accept?";

	    if(H2Config::MyColor() != GetColor() && AI::JoinTroops(*this, monster, count))
	    {
		tile.SetQuantity1(0);
		tile.SetQuantity2(0);
	    }
	    else
	    if(Dialog::YES == Dialog::Message(Monster::String(monster), message, Font::BIG, Dialog::YES|Dialog::NO))
	    {
	    	JoinTroops(monster, count);
		tile.SetQuantity1(0);
		tile.SetQuantity2(0);

		// redraw status info
		Game::StatusWindow::Get().Redraw();
	    }
	}
	// is full
	else
	if(H2Config::MyColor() == GetColor())
	    Dialog::Message(Monster::String(monster), "You are unable to recruit at this time, your ranks are full.", Font::BIG, Dialog::OK);
    }
    // is void
    else
    if(H2Config::MyColor() == GetColor())
	    Dialog::Message(Monster::String(monster), "As you approach the dwelling, you notice that there is no one here.", Font::BIG, Dialog::OK);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToAcceptArmy: " + GetName() + ", object: " + std::string(MP2::StringObject(obj)));
}

void Heroes::ActionToRecruitArmy(const u16 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const MP2::object_t obj = tile.GetObject();
    
    std::string msg_full, msg_void, msg_attk;

    switch(obj)
    {
        case MP2::OBJ_RUINS:
    	    msg_void = "You search the ruins, but the Medusas that used to live here are gone. Perhaps there will be more next week.";
    	    msg_full = "You've found some Medusas living in the ruins. They are willing to join your army for a price. Do you want to recruit Medusas?";
    	    break;

        case MP2::OBJ_TREECITY:
    	    msg_void = "You've found a Sprite Tree City. Unfortunately, none of the Sprites living there wish to join an army. Maybe next week.";
    	    msg_full = "Some of the Sprites living in the tree city are willing to join your army for a price. Do you want to recruit Sprites?";
    	    break;
  
        case MP2::OBJ_WAGONCAMP:
    	    msg_void = "A colorful Rogues' wagon stands empty here. Perhaps more Rogues will be here later.";
    	    msg_full = "Distant sounds of music and laughter draw you to a colorful wagon housing Rogues. Do you wish to have any Rogues join your army?";
    	    break;

        case MP2::OBJ_TROLLBRIDGE:
    	    msg_void = "You've found one of those bridges that Trolls are so fond of living under, but there are none here. Perhaps there will be some next week.";
    	    msg_full = "Some Trolls living under a bridge are willing to join your army, but for a price. Do you want to recruit Trolls?";
	    // UNKNOWN MESS: "A few Trolls remain, cowering under the bridge. They approach you and offer to join your forces as mercenaries. Do you want to buy any Trolls?";
    	    msg_attk = "Trolls living under the bridge challenge you. Will you fight them?";
    	    break;

	case MP2::OBJ_DESERTTENT:
    	    msg_void = "A group of tattered tents, billowing in the sandy wind, beckons you. The tents are unoccupied. Perhaps more Nomads will be here later.";
    	    msg_full = "A group of tattered tents, billowing in the sandy wind, beckons you. Do you wish to have any Nomads join you during your travels?";
    	    break;

	default: return;
    }
    const Monster::monster_t monster = Monster::Monster(obj);
    const u32 count = tile.GetQuantity2() * 0xFF + tile.GetQuantity1();
    const u32 army_size = GetCountArmy();

    if(count)
    {
	if(((HEROESMAXARMY == army_size && HasMonster(monster)) || HEROESMAXARMY > army_size))
	{
	    if(H2Config::MyColor() != GetColor())
	    {
		const u16 recruit = AI::RecruitTroops(*this, monster, count);
		tile.SetQuantity1((count - recruit) % 0xFF);
		tile.SetQuantity2((count - recruit) / 0xFF);
	    }
	    else
	    if(Dialog::YES == Dialog::Message(Monster::String(monster), msg_full, Font::BIG, Dialog::YES|Dialog::NO))
	    {
		// recruit
		const u16 recruit = Dialog::RecruitMonster(monster, count);
		if(recruit)
		{
	    	    JoinTroops(monster, recruit);
		    tile.SetQuantity1((count - recruit) % 0xFF);
		    tile.SetQuantity2((count - recruit) / 0xFF);

		    // redraw status info
		    Game::StatusWindow::Get().Redraw();
		}
	    }
	}
	// is full
	else
	if(H2Config::MyColor() == GetColor())
	    Dialog::Message(Monster::String(monster), "You are unable to recruit at this time, your ranks are full.", Font::BIG, Dialog::OK);
    }
    // is void
    else
	if(H2Config::MyColor() == GetColor())
	    Dialog::Message(Monster::String(monster), msg_void, Font::BIG, Dialog::OK);

    if(H2Config::Debug()) Error::Verbose("Heroes::ActionToRecruitArmy: " + GetName() + ", object: " + std::string(MP2::StringObject(obj)));
}
