/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
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

#include "dialog.h"
#include "agg.h"
#include "button.h"
#include "cursor.h"
#include "difficulty.h"
#include "settings.h"
#include "maps.h"
#include "text.h"
#include "tools.h"
#include "dialog_selectscenario.h"

void LossConditionInfo(const Maps::FileInfo &);
void VictoryConditionInfo(const Maps::FileInfo &);

void ScenarioListBox::RedrawItem(const Maps::FileInfo & info, u16 dstx, u16 dsty, bool current)
{
    Display & display = Display::Get();
    Text text;
    u8 index = 19 + info.KingdomColorsCount();

    if(!Settings::Get().QVGA())
    {
	dstx = dstx - 10;
	dsty = dsty + 2;
    }

    const Sprite & spriteCount = AGG::GetICN(ICN::REQUESTS, index);
    display.Blit(spriteCount, dstx, dsty);

    switch(info.size_w)
    {
        case Maps::SMALL:	index = 26; break;
        case Maps::MEDIUM:	index = 27; break;
        case Maps::LARGE:	index = 28; break;
        case Maps::XLARGE:	index = 29; break;
        default:  		index = 30; break;
    }

    const Sprite & spriteSize = AGG::GetICN(ICN::REQUESTS, index);
    display.Blit(spriteSize, dstx + spriteCount.w() + 2, dsty);

    text.Set(info.name, (current ? Font::YELLOW_BIG : Font::BIG));
    text.Blit(dstx + 54, dsty + 2);

    index = 30 + info.conditions_wins;
    const Sprite & spriteWins = AGG::GetICN(ICN::REQUESTS, index);
    display.Blit(spriteWins, dstx + 224, dsty);

    index = 36 + info.conditions_loss;
    const Sprite & spriteLoss = AGG::GetICN(ICN::REQUESTS, index);
    display.Blit(spriteLoss, dstx + 224 + spriteWins.w() + 2, dsty);
}

void ScenarioListBox::RedrawBackground(const Point & dst)
{
    Display & display = Display::Get();

    if(Settings::Get().QVGA())
    {
	display.Blit(AGG::GetICN(ICN::STONEBAK, 0), Rect(17, 37, 266, 156), dst.x + 15, dst.y + 35);
	display.Blit(AGG::GetICN(ICN::REQSBKG, 0), Rect(325, 70, 16, 100), dst.x + 283, dst.y + 55);
	display.Blit(AGG::GetICN(ICN::REQSBKG, 0), Rect(325, 167, 16, 50), dst.x + 283, dst.y + 125);
    }
    else
    {
	display.Blit(AGG::GetICN(ICN::REQSBKG, 0), dst);

	if(content && cur != content->end())
	{
	    Text text;
	    const Maps::FileInfo & info = *cur;
	    u8 index = 19 + info.KingdomColorsCount();

	    const Sprite & spriteCount = AGG::GetICN(ICN::REQUESTS, index);
	    display.Blit(spriteCount, dst.x + 65, dst.y + 265);

	    switch(info.size_w)
	    {
		case Maps::SMALL:	index = 26; break;
    		case Maps::MEDIUM:	index = 27; break;
    		case Maps::LARGE:	index = 28; break;
    		case Maps::XLARGE:	index = 29; break;
    		default:  		index = 30; break;
	    }

	    const Sprite & spriteSize = AGG::GetICN(ICN::REQUESTS, index);
	    display.Blit(spriteSize, dst.x + 65 + spriteCount.w() + 2, dst.y + 265);

	    text.Set(info.name, Font::BIG);
	    text.Blit(dst.x + 190 - text.w() / 2, dst.y + 265);

	    index = 30 + info.conditions_wins;
	    const Sprite & spriteWins = AGG::GetICN(ICN::REQUESTS, index);
	    display.Blit(spriteWins, dst.x + 275, dst.y + 265);

	    index = 36 + info.conditions_loss;
	    const Sprite & spriteLoss = AGG::GetICN(ICN::REQUESTS, index);
	    display.Blit(spriteLoss, dst.x + 275 + spriteWins.w() + 2, dst.y + 265);

	    text.Set(_("Maps Difficulty:"), Font::BIG);
	    text.Blit(dst.x + 70, dst.y + 290);

	    text.Set(Difficulty::String(info.difficulty));
	    text.Blit(dst.x + 275 - text.w() / 2, dst.y + 290);
    
	    TextBox box(info.description, Font::BIG, 290);
	    box.Blit(dst.x + 45, dst.y + 320);
	}
    }
}

void ScenarioListBox::ActionCurrentUp(void)
{
}

void ScenarioListBox::ActionCurrentDn(void)
{
}

void ScenarioListBox::ActionListDoubleClick(Maps::FileInfo &)
{
}

void ScenarioListBox::ActionListSingleClick(Maps::FileInfo &)
{
}

bool Dialog::SelectScenario(const MapsFileInfoList & all, std::string & filename)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    MapsFileInfoList small;
    MapsFileInfoList medium;
    MapsFileInfoList large;
    MapsFileInfoList xlarge;

    small.reserve(all.size());
    medium.reserve(all.size());
    large.reserve(all.size());
    xlarge.reserve(all.size());

    for(MapsFileInfoList::const_iterator cur = all.begin(); cur != all.end(); ++ cur)
    {
	switch((*cur).size_w)
	{
    	    case Maps::SMALL:	small.push_back(*cur); break;
    	    case Maps::MEDIUM:	medium.push_back(*cur); break;
    	    case Maps::LARGE:	large.push_back(*cur); break;
    	    case Maps::XLARGE:	xlarge.push_back(*cur); break;
	    default: continue;
	}
    }

    const Sprite & panel = AGG::GetICN(ICN::REQSBKG, 0);
    Background back((display.w() - panel.w()) / 2, (display.h() - panel.h()) / 2, panel.w(), panel.h());
    back.Save();

    const Rect & rt = back.GetRect();

    const Rect countPlayers(rt.x + 45, rt.y + 55, 20, 175);
    const Rect sizeMaps(rt.x + 62, rt.y + 55, 20, 175);
    const Rect victoryConds(rt.x + 267, rt.y + 55, 20, 175);
    const Rect lossConds(rt.x + 287, rt.y + 55, 20, 175);

    const Rect curCountPlayer(rt.x + 66, rt.y + 264, 18, 18);
    const Rect curMapSize(rt.x + 85, rt.y + 264, 18, 18);
    const Rect curMapName(rt.x + 107, rt.y + 264, 166, 18);
    const Rect curVictoryCond(rt.x + 277, rt.y + 264, 18, 18);
    const Rect curLossCond(rt.x + 295, rt.y + 264, 18, 18);
    const Rect curDifficulty(rt.x + 220, rt.y + 292, 114, 20);
    const Rect curDescription(rt.x + 42, rt.y + 316, 292, 90);

    Button buttonOk(rt.x + 140, rt.y + 410, ICN::REQUESTS, 1, 2);

    Button buttonSelectSmall(rt.x + 37, rt.y + 22, ICN::REQUESTS, 9, 10);
    Button buttonSelectMedium(rt.x + 99, rt.y + 22, ICN::REQUESTS, 11, 12);
    Button buttonSelectLarge(rt.x + 161, rt.y + 22, ICN::REQUESTS, 13, 14);
    Button buttonSelectXLarge(rt.x + 223, rt.y + 22, ICN::REQUESTS, 15, 16);
    Button buttonSelectAll(rt.x + 285, rt.y + 22, ICN::REQUESTS, 17, 18);

    if(small.empty()) buttonSelectSmall.SetDisable(true);
    if(medium.empty()) buttonSelectMedium.SetDisable(true);
    if(large.empty()) buttonSelectLarge.SetDisable(true);
    if(xlarge.empty()) buttonSelectXLarge.SetDisable(true);

    ScenarioListBox listbox(rt);

    listbox.RedrawBackground(rt);
    listbox.SetScrollButtonUp(ICN::REQUESTS, 5, 6, Point(rt.x + 327, rt.y + 55));
    listbox.SetScrollButtonDn(ICN::REQUESTS, 7, 8, Point(rt.x + 327, rt.y + 217));
    listbox.SetScrollSplitter(AGG::GetICN(ICN::ESCROLL, 3), Rect(rt.x + 330, rt.y + 73, 12, 141));
    listbox.SetAreaMaxItems(9);
    listbox.SetAreaItems(Rect(rt.x + 55, rt.y + 55, 270, 175));
    listbox.SetListContent(const_cast<MapsFileInfoList &>(all));
    listbox.Redraw();

    buttonOk.Draw();
    buttonSelectMedium.Draw();
    buttonSelectLarge.Draw();
    buttonSelectXLarge.Draw();
    buttonSelectAll.Draw();

    filename.clear();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(buttonOk.isEnable()) le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
	le.MousePressLeft(buttonSelectSmall) && buttonSelectSmall.isEnable() ? buttonSelectSmall.PressDraw() : buttonSelectSmall.ReleaseDraw();
	le.MousePressLeft(buttonSelectMedium) && buttonSelectMedium.isEnable() ? buttonSelectMedium.PressDraw() : buttonSelectMedium.ReleaseDraw();
	le.MousePressLeft(buttonSelectLarge) && buttonSelectLarge.isEnable() ? buttonSelectLarge.PressDraw() : buttonSelectLarge.ReleaseDraw();
	le.MousePressLeft(buttonSelectXLarge) && buttonSelectXLarge.isEnable() ? buttonSelectXLarge.PressDraw() : buttonSelectXLarge.ReleaseDraw();
	le.MousePressLeft(buttonSelectAll) ? buttonSelectAll.PressDraw() : buttonSelectAll.ReleaseDraw();

        if((buttonOk.isEnable() && le.MouseClickLeft(buttonOk)) || le.KeyPress(KEY_RETURN))
	{
	    filename = listbox.GetCurrent().file;
	    break;
	}
	else
        if(le.KeyPress(KEY_ESCAPE))
	{
	    filename.clear();
	    break;
	}
	else
	if(((le.MouseClickLeft(buttonSelectSmall) || le.KeyPress(KEY_s)) && buttonSelectSmall.isEnable()) && buttonSelectSmall.isEnable())
	{
	    listbox.SetListContent(small);
	    cursor.Hide();
	}
	else
	if(((le.MouseClickLeft(buttonSelectMedium) || le.KeyPress(KEY_m)) && buttonSelectMedium.isEnable()) && buttonSelectMedium.isEnable())
	{
	    listbox.SetListContent(medium);
	    cursor.Hide();
	}
	else
	if(((le.MouseClickLeft(buttonSelectLarge) || le.KeyPress(KEY_l)) && buttonSelectLarge.isEnable()) && buttonSelectLarge.isEnable())
	{
	    listbox.SetListContent(large);
	    cursor.Hide();
	}
	else
	if(((le.MouseClickLeft(buttonSelectXLarge) || le.KeyPress(KEY_x)) && buttonSelectXLarge.isEnable()) && buttonSelectXLarge.isEnable())
	{
	    listbox.SetListContent(xlarge);
	    cursor.Hide();
	}
	else
	if(le.MouseClickLeft(buttonSelectAll) || le.KeyPress(KEY_a))
	{
	    listbox.SetListContent(const_cast<MapsFileInfoList &>(all));
	    cursor.Hide();
	}

	listbox.QueueEventProcessing();

	// right info
	if(le.MousePressRight(buttonSelectSmall)) Dialog::Message(_("Small Maps"), _("View only maps of size small (36x36)."), Font::BIG);
	else
	if(le.MousePressRight(buttonSelectMedium)) Dialog::Message(_("Medium Maps"), _("View only maps of size medium (72x72)."), Font::BIG);
	else
	if(le.MousePressRight(buttonSelectLarge)) Dialog::Message(_("Large Maps"), _("View only maps of size large (108x108)."), Font::BIG);
	else
	if(le.MousePressRight(buttonSelectXLarge)) Dialog::Message(_("Extra Large Maps"), _("View only maps of size extra large (144x144)."), Font::BIG);
	else
	if(le.MousePressRight(buttonSelectAll)) Dialog::Message(_("All Maps"), _("View all maps, regardless of size."), Font::BIG);
	else
	if(le.MousePressRight(countPlayers) || le.MousePressRight(curCountPlayer)) Dialog::Message(_("Players Icon"), _("Indicates how many players total are in the EditScenario. Any positions not occupied by humans will be occupied by computer players."), Font::BIG);
	else
	if(le.MousePressRight(sizeMaps) || le.MousePressRight(curMapSize)) Dialog::Message(_("Size Icon"), _("Indicates whether the maps is small (36x36), medium (72x72), large (108x108), or extra large (144x144)."), Font::BIG);
	else
	if(le.MousePressRight(curMapName)) Dialog::Message(_("Selected Name"), _("The name of the currently selected map."), Font::BIG);
	else
	if(le.MousePressRight(victoryConds))
	{
	    const Maps::FileInfo* item = listbox.GetFromPosition(le.GetMouseCursor());
	    if(item) VictoryConditionInfo(*item);
	}
	else
	if(le.MousePressRight(lossConds))
	{
	    const Maps::FileInfo* item = listbox.GetFromPosition(le.GetMouseCursor());
	    if(item) LossConditionInfo(*item);
	}
	else
	if(le.MousePressRight(curVictoryCond)) VictoryConditionInfo(listbox.GetCurrent());
	else
	if(le.MousePressRight(curLossCond)) LossConditionInfo(listbox.GetCurrent());
	else
	if(le.MousePressRight(curDifficulty)) Dialog::Message(_("Selected Map Difficulty"), _("The map difficulty of the currently selected map.  The map difficulty is determined by the EditScenario designer. More difficult maps might include more or stronger enemies, fewer resources, or other special conditions making things tougher for the human player."), Font::BIG);
	else
	if(le.MousePressRight(curDescription)) Dialog::Message(_("Selected Description"), _("The description of the currently selected map."), Font::BIG);
	else
	if(le.MousePressRight(buttonOk)) Dialog::Message(_("OK"), _("Accept the choice made."), Font::BIG);

	if(!cursor.isVisible())
	{
	    listbox.Redraw();
	    buttonOk.Draw();
    	    buttonSelectSmall.Draw();
    	    buttonSelectMedium.Draw();
    	    buttonSelectLarge.Draw();
    	    buttonSelectXLarge.Draw();
    	    buttonSelectAll.Draw();
	    cursor.Show();
	    display.Flip();
	}
    }

    cursor.Hide();
    back.Restore();

    return filename.size();
}

void LossConditionInfo(const Maps::FileInfo & info)
{
    std::string msg;

    switch(info.conditions_loss)
    {
	case 0:	msg = _("Lose all your heroes and towns."); break;
	case 1:	msg = _("Lose a specific town."); break;
	case 2:	msg = _("Lose a specific hero."); break;
	case 3:	msg = _("Run out of time. Fail to win by a certain point."); break;
	default: return;
    }
    Dialog::Message(_("Loss Condition"), msg, Font::BIG);
}

void VictoryConditionInfo(const Maps::FileInfo & info)
{
    std::string msg;

    switch(info.conditions_wins)
    {
	case 0:	msg = _("Defeat all enemy heroes and towns."); break;
	case 1:	msg = _("Capture a specific town."); break;
	case 2:	msg = _("Defeat a specific hero."); break;
	case 3:	msg = _("Find a specific artifact."); break;
	case 4:	msg = _("Your side defeats the opposing side."); break;
	case 5:	msg = _("Accumulate a large amount of gold."); break;
	default: return;
    }
    Dialog::Message(_("Victory Condition"), msg, Font::BIG);
}
