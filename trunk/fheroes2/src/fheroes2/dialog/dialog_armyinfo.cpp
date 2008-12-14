/***************************************************************************
 *   Copyright (C) 2006 by Andrey Afletdinov                               *
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
#include "button.h"
#include "cursor.h"
#include "settings.h"
#include "monster.h"
#include "morale.h"
#include "luck.h"
#include "army.h"
#include "skill.h"
#include "dialog.h"
#include "game.h"
#include "battle_troop.h"

Dialog::answer_t Dialog::ArmyInfo(const Army::Troop & troops, u16 flags)
{
    Army::BattleTroop battroop(troops);
    return ArmyInfo(battroop, flags);
}

Dialog::answer_t Dialog::ArmyInfo(const Army::BattleTroop & troop, u16 flags)
{
    Army::BattleTroop battroop(troop);
    const bool battle = BATTLE & flags;
    Display & display = Display::Get();

    const ICN::icn_t viewarmy = H2Config::EvilInterface() ? ICN::VIEWARME : ICN::VIEWARMY;

    const Surface & sprite_dialog = AGG::GetICN(viewarmy, 0);

    const Monster::stats_t stats = Monster::GetStats(battroop.Monster());
    const Skill::Primary *skills = battroop.MasterSkill();

    Rect pos_rt;

    pos_rt.x = (display.w() - sprite_dialog.w()) / 2;
    pos_rt.y = (display.h() - sprite_dialog.h()) / 2;
    pos_rt.w = sprite_dialog.w();
    pos_rt.h = sprite_dialog.h();

    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    Background back(pos_rt);
    
    back.Save();

    display.Blit(sprite_dialog, pos_rt.x, pos_rt.y);

    Point dst_pt(pos_rt.x, pos_rt.y);
    std::string message;

    // name
    dst_pt.x = pos_rt.x  + 140 - Text::width(stats.name, Font::BIG) / 2;
    dst_pt.y = pos_rt.y + 40;
    Text(stats.name, Font::BIG, dst_pt);
    
    // count
    String::AddInt(message, battroop.Count());
    dst_pt.x = pos_rt.x + 140 - Text::width(message, Font::BIG) / 2;
    dst_pt.y = pos_rt.y + 225;
    Text(message, Font::BIG, dst_pt);
    
    // attack
    message = "Attack:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y = pos_rt.y + 40;
    Text(message, Font::BIG, dst_pt);

    message.clear();
    String::AddInt(message, stats.attack);

    if(skills)
    {
	message += " (";
	String::AddInt(message, stats.attack + (*skills).GetAttack());
	message += ")";
    }

    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // defense
    message = "Defense:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message.clear();
    String::AddInt(message, stats.defence);

    if(skills)
    {
	message += " (";
	String::AddInt(message, stats.defence + (*skills).GetDefense());
	message += ")";
    }

    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // shot
    if(stats.shots) {
	message = battle ? "Shots Left:" : "Shots:";
	dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
	dst_pt.y += 18;
	Text(message, Font::BIG, dst_pt);

	message.clear();
	String::AddInt(message, battle ? battroop.shots : stats.shots);
	dst_pt.x = pos_rt.x + 420;
	Text(message, Font::BIG, dst_pt);
    }

    // damage
    message = "Damage:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message.clear();
    String::AddInt(message, stats.damageMin);
    message += " - ";
    String::AddInt(message, stats.damageMax);
    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // hp
    message = "Hit Points:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message.clear();
    String::AddInt(message, stats.hp);
    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    if(battle) {
	message = "Hit Points Left:";
	dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
	dst_pt.y += 18;
	Text(message, Font::BIG, dst_pt);
	
	message.clear();
	String::AddInt(message, battroop.hp);
	dst_pt.x = pos_rt.x + 420;
	Text(message, Font::BIG, dst_pt);
    }

    // speed
    message = "Speed:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message = Speed::String(stats.speed);
    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // morale
    message = "Morale:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message = (skills ? Morale::String((*skills).GetMorale()) : Morale::String(Morale::NORMAL));
    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // luck
    message = "Luck:";
    dst_pt.x = pos_rt.x + 400 - Text::width(message, Font::BIG);
    dst_pt.y += 18;
    Text(message, Font::BIG, dst_pt);

    message = (skills ? Luck::String((*skills).GetLuck()) : Luck::String(Luck::NORMAL));
    dst_pt.x = pos_rt.x + 420;
    Text(message, Font::BIG, dst_pt);

    // button upgrade
    dst_pt.x = pos_rt.x + 284;
    dst_pt.y = pos_rt.y + 190;
    Button buttonUpgrade(dst_pt, viewarmy, 5, 6);

    // button dismiss
    dst_pt.x = pos_rt.x + 284;
    dst_pt.y = pos_rt.y + 222;
    Button buttonDismiss(dst_pt, viewarmy, 1, 2);

    // button exit
    dst_pt.x = pos_rt.x + 410;
    dst_pt.y = pos_rt.y + 222;
    Button buttonExit(dst_pt, viewarmy, 3, 4);

    if(READONLY & flags)
    {
	buttonDismiss.Press();
	buttonDismiss.SetDisable(true);
    }

    if(Monster::AllowUpgrade(stats.monster))
    {
	if(UPGRADE & flags)
	{
	    buttonUpgrade.SetDisable(false);
	    buttonUpgrade.Draw();
	}
	else
	if(READONLY & flags)
	{
	    buttonUpgrade.Press();
	    buttonUpgrade.SetDisable(true);
	    buttonUpgrade.Draw();
	}
    }
    else
	buttonUpgrade.SetDisable(true);

    if(BUTTONS & flags)
    {
	buttonDismiss.Draw();
	buttonExit.Draw();
    }

    LocalEvent & le = LocalEvent::GetLocalEvent();
    
    Dialog::answer_t result = Dialog::ZERO;

    const Sprite & frame = AGG::GetICN(stats.file_icn, 0);
    Point anim_rt(pos_rt.x + (pos_rt.w / 2 - frame.w()) / 2 , pos_rt.y + 180);
    battroop.astate = Monster::AS_IDLE;
    battroop.aframe = 0;
    battroop.Animate();
    battroop.Blit(anim_rt, battroop.IsReflected());

    cursor.Show();
    display.Flip();
    
    // dialog menu loop
    while(le.HandleEvents())
    {
        if(flags & BUTTONS)
        {
            if(buttonUpgrade.isEnable()) le.MousePressLeft(buttonUpgrade) ? (buttonUpgrade).PressDraw() : (buttonUpgrade).ReleaseDraw();
    	    if(buttonDismiss.isEnable()) le.MousePressLeft(buttonDismiss) ? (buttonDismiss).PressDraw() : (buttonDismiss).ReleaseDraw();
    	    le.MousePressLeft(buttonExit) ? (buttonExit).PressDraw() : (buttonExit).ReleaseDraw();
            
            // upgrade
            if(buttonUpgrade.isEnable() && le.MouseClickLeft(buttonUpgrade)){ result = Dialog::UPGRADE; break; }
            
    	    // dismiss
            if(buttonDismiss.isEnable() && le.MouseClickLeft(buttonDismiss)){ result = Dialog::DISMISS; break; }
            
    	    // exit
    	    if(le.MouseClickLeft(buttonExit) || le.KeyPress(KEY_ESCAPE)){ result = Dialog::CANCEL; break; }
        }
        else
        {
            if(!le.MouseRight()) break;
        }
    }

    cursor.Hide();
    back.Restore();
    return result;
}
