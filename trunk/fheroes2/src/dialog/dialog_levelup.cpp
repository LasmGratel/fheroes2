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
#include "config.h"
#include "cursor.h"
#include "text.h"
#include "localevent.h"
#include "button.h"
#include "dialog.h"

Skill::Secondary::skill_t Dialog::LevelUpSelectSkill(const std::string & header, const Skill::Secondary & sec1, const Skill::Secondary & sec2)
{
    Display & display = Display::Get();
    const ICN::icn_t system = H2Config::EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    // preload
    AGG::PreloadObject(system);

    // cursor
    Cursor & cursor = Cursor::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite_frame = AGG::GetICN(ICN::SECSKILL, 15);
    const Sprite & sprite_skill1 = AGG::GetICN(ICN::SECSKILL, Skill::Secondary::GetIndexSprite1(sec1.Skill()));
    const Sprite & sprite_skill2 = AGG::GetICN(ICN::SECSKILL, Skill::Secondary::GetIndexSprite1(sec2.Skill()));

    Point pt;
    const std::string message("You may learn either " + Skill::Level::String(sec1.Level()) + " " + Skill::Secondary::String(sec1.Skill()) + " or " + Skill::Level::String(sec2.Level()) + " " + Skill::Secondary::String(sec2.Skill()) + ".");
    Box box(Text::height(header, Font::BIG, BOXAREA_WIDTH) + 20 + Text::height(message, Font::BIG, BOXAREA_WIDTH) + 20 + sprite_frame.h(), true);

    pt.x = box.GetArea().x + box.GetArea().w / 2 - AGG::GetICN(system, 9).w() - 20;
    pt.y = box.GetArea().y + box.GetArea().h + BUTTON_HEIGHT - AGG::GetICN(system, 9).h();
    Button button_learn1(pt, system, 9, 10);

    pt.x = box.GetArea().x + box.GetArea().w / 2 + 20;
    pt.y = box.GetArea().y + box.GetArea().h + BUTTON_HEIGHT - AGG::GetICN(system, 9).h();
    Button button_learn2(pt, system, 9, 10);

    Rect pos = box.GetArea();

    if(header.size())
    {
	TextBox(header, Font::BIG, pos);
        pos.y += Text::height(header, Font::BIG, BOXAREA_WIDTH) + 20;
    }

    if(message.size())
    {
        TextBox(message, Font::BIG, pos);
        pos.y += Text::height(message, Font::BIG, BOXAREA_WIDTH) + 20;
    }

    // sprite1
    pos.y += 20;
    pos.x = box.GetArea().x + box.GetArea().w / 2 - sprite_frame.w() - 20;
    display.Blit(sprite_frame, pos);
    pos.x += 3;
    Rect rect_image1(pos, sprite_skill1.w(), sprite_skill1.h());
    display.Blit(sprite_skill1, pos.x, pos.y + 3);
    // text
    const std::string &name_skill1 = Skill::Secondary::String(sec1.Skill());
    Text(name_skill1, Font::SMALL, pos.x + (sprite_skill1.w() - Text::width(name_skill1, Font::SMALL)) / 2, pos.y + 5);
    const std::string &name_level1 = Skill::Level::String(sec1.Level());
    Text(name_level1, Font::SMALL, pos.x + (sprite_skill1.w() - Text::width(name_level1, Font::SMALL)) / 2, pos.y + sprite_skill1.h() - 12);

    // sprite2
    pos.x = box.GetArea().x + box.GetArea().w / 2 + 20;
    display.Blit(sprite_frame, pos);
    pos.x += 3;
    Rect rect_image2(pos, sprite_skill2.w(), sprite_skill2.h());
    display.Blit(sprite_skill2, pos.x, pos.y + 3);
    // text
    const std::string &name_skill2 = Skill::Secondary::String(sec2.Skill());
    Text(name_skill2, Font::SMALL, pos.x + (sprite_skill2.w() - Text::width(name_skill2, Font::SMALL)) / 2, pos.y + 5);
    const std::string &name_level2 = Skill::Level::String(sec2.Level());
    Text(name_level2, Font::SMALL, pos.x + (sprite_skill2.w() - Text::width(name_level2, Font::SMALL)) / 2, pos.y + sprite_skill2.h() - 12);

    button_learn1.Draw();
    button_learn2.Draw();

    cursor.Show();
    display.Flip();
    LocalEvent & le = LocalEvent::GetLocalEvent();
    Skill::Secondary::skill_t result = Skill::Secondary::UNKNOWN;

    // message loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(button_learn1) ? button_learn1.PressDraw() : button_learn1.ReleaseDraw();
	le.MousePressLeft(button_learn2) ? button_learn2.PressDraw() : button_learn2.ReleaseDraw();

        if(le.MouseClickLeft(button_learn1)){ result = sec1.Skill(); break; }
        if(le.MouseClickLeft(button_learn2)){ result = sec2.Skill(); break; }

	if(le.MouseClickLeft(rect_image1)){ cursor.Hide(); SkillInfo(sec1.Skill(), sec1.Level()); cursor.Show(); display.Flip(); }
	else
	if(le.MouseClickLeft(rect_image2)){ cursor.Hide(); SkillInfo(sec2.Skill(), sec2.Level()); cursor.Show(); display.Flip(); }

	if(le.MousePressRight(rect_image1)){ cursor.Hide(); SkillInfo(sec1.Skill(), sec1.Level(), false); cursor.Show(); display.Flip(); }
	else
	if(le.MousePressRight(rect_image2)){ cursor.Hide(); SkillInfo(sec2.Skill(), sec2.Level(), false); cursor.Show(); display.Flip(); }
    }

    cursor.Hide();

    return result;
}
