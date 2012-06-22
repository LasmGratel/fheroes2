/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "agg.h"
#include "settings.h"
#include "world.h"
#include "maps.h"
#include "game.h"
#include "game_focus.h"
#include "game_interface.h"
#include "route.h"
#include "interface_gamearea.h"

#define SCROLL_MIN	8
#define SCROLL_MAX	TILEWIDTH

namespace Game
{
    // game_startgame.cpp
    Cursor::themes_t GetCursor(s32);
    void MouseCursorAreaClickLeft(s32);
    void MouseCursorAreaPressRight(s32);
}

Interface::GameArea & Interface::GameArea::Get(void)
{
    static Interface::GameArea ga;
    return ga;
}

Interface::GameArea::GameArea() : oldIndexPos(0), updateCursor(false)
{
}

const Rect & Interface::GameArea::GetArea(void) const
{ return areaPosition; }

const Point& Interface::GameArea::GetMapsPos(void) const
{ return rectMapsPosition; }

const Rect & Interface::GameArea::GetRectMaps(void) const
{ return rectMaps; }

/* fixed src rect image */
void Interface::GameArea::SrcRectFixed(Rect & src, Point & dst, const u16 rw, const u16 rh)
{
    const Rect & areaPosition = Interface::GameArea::Get().GetArea();
    ToolsSrcRectFixed(src, dst.x, dst.y, rw, rh, areaPosition);
}

void Interface::GameArea::SrcRectFixed(Rect & src, s16 & dst_x, s16 & dst_y, const u16 rw, const u16 rh)
{
    const Rect & areaPosition = Interface::GameArea::Get().GetArea();
    ToolsSrcRectFixed(src, dst_x, dst_y, rw, rh, areaPosition);
}

void Interface::GameArea::Build(void)
{
    if(Settings::Get().ExtGameHideInterface())
	SetAreaPosition(0, 0,
		    Display::Get().w(),
		    Display::Get().h());
    else
	SetAreaPosition(BORDERWIDTH, BORDERWIDTH,
		    Display::Get().w() - RADARWIDTH - 3 * BORDERWIDTH,
		    Display::Get().h() - 2 * BORDERWIDTH);
}

void Interface::GameArea::SetAreaPosition(s16 x, s16 y, u16 w, u16 h)
{
    areaPosition.x = x;
    areaPosition.y = y;
    areaPosition.w = w;
    areaPosition.h = h;

    rectMaps.x = 0;
    rectMaps.y = 0;

    rectMaps.w = (areaPosition.w / TILEWIDTH) + 2;
    rectMaps.h = (areaPosition.h / TILEWIDTH) + 2;

    scrollOffset.x = 0;
    scrollOffset.y = 0;
    scrollStepX = Settings::Get().ScrollSpeed();
    scrollStepY = Settings::Get().ScrollSpeed();

    if(world.w() < rectMaps.w)
    {
	rectMaps.w = (areaPosition.w / TILEWIDTH);
	scrollStepX = SCROLL_MAX;
    }

    if(world.h() < rectMaps.h)
    {
	rectMaps.h = (areaPosition.h / TILEWIDTH);
	scrollStepY = SCROLL_MAX;
    }

    if(Settings::Get().Editor())
    {
	rectMaps.w = (areaPosition.w / TILEWIDTH);
	rectMaps.h = (areaPosition.h / TILEWIDTH);
	scrollStepX = SCROLL_MAX;
	scrollStepY = SCROLL_MAX;
    }

    tailX = areaPosition.w - TILEWIDTH * (areaPosition.w / TILEWIDTH);
    tailY = areaPosition.h - TILEWIDTH * (areaPosition.h / TILEWIDTH);

    rectMapsPosition.x = areaPosition.x - scrollOffset.x;
    rectMapsPosition.y = areaPosition.y - scrollOffset.y;
}

void Interface::GameArea::BlitOnTile(Surface & dst, const Sprite & src, const Point & mp) const
{
    BlitOnTile(dst, src, src.x(), src.y(), mp);
}

void Interface::GameArea::BlitOnTile(Surface & dst, const Surface & src, const s16 ox, const s16 oy, const Point & mp) const
{
    const s16 & dstx = rectMapsPosition.x + TILEWIDTH * (mp.x - rectMaps.x);
    const s16 & dsty = rectMapsPosition.y + TILEWIDTH * (mp.y - rectMaps.y);

    Point dstpt(dstx + ox, dsty + oy);

    if(areaPosition & Rect(dstpt, src.w(), src.h()))
    {
	Rect srcrt;
	SrcRectFixed(srcrt, dstpt, src.w(), src.h());
	src.Blit(srcrt, dstpt, dst);
    }
}

void Interface::GameArea::Redraw(Surface & dst, u8 flag) const
{
    return Redraw(dst, flag, Rect(0, 0, rectMaps.w, rectMaps.h));
}

void Interface::GameArea::Redraw(Surface & dst, u8 flag, const Rect & rt) const
{
    // tile
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawTile(dst);

    // bottom
    if(flag & LEVEL_BOTTOM)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawBottom(dst, !(flag & LEVEL_OBJECTS));

    // ext object
    if(flag & LEVEL_OBJECTS)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawObjects(dst);

    // top
    if(flag & LEVEL_TOP)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawTop(dst);

    // heroes
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
    {
	const Maps::Tiles & tile = world.GetTiles(rectMaps.x + ox, rectMaps.y + oy);

	if(tile.GetObject() == MP2::OBJ_HEROES && (flag & LEVEL_HEROES))
	{
	    const Heroes* hero = tile.GetHeroes();
	    if(hero) hero->Redraw(dst, rectMapsPosition.x + TILEWIDTH * ox, rectMapsPosition.y + TILEWIDTH * oy, true);
	}
    }

    // route
    const Heroes* hero = flag & LEVEL_HEROES ? GameFocus::GetHeroes() : NULL;

    if(hero && hero->GetPath().isShow())
    {
	s32 from = hero->GetIndex();
	s16 green = hero->GetPath().GetAllowStep();

	const bool skipfirst = hero->isEnableMove() && 45 > hero->GetSpriteIndex() && 2 < (hero->GetSpriteIndex() % 9);

	Route::Path::const_iterator it1 = hero->GetPath().begin();
	Route::Path::const_iterator it2 = hero->GetPath().end();
	Route::Path::const_iterator it3 = it1;

	for(; it1 != it2; ++it1)
	{
	    from = (*it1).GetIndex();
    	    const Point mp = Maps::GetPoint(from);

 	    ++it3;
	    --green;

	    // is visible
            if((Rect(rectMaps.x + rt.x, rectMaps.y + rt.y, rt.w, rt.h) & mp) &&
	    // check skip first?
	       ! (it1 == hero->GetPath().begin() && skipfirst))
	    {
		const u16 index = (it3 == it2 ? 0 :
		    Route::Path::GetIndexSprite((*it1).GetDirection(), (*it3).GetDirection(), 
			Maps::Ground::GetPenalty(from, Direction::CENTER, hero->GetLevelSkill(Skill::Secondary::PATHFINDING))));

		const Sprite & sprite = AGG::GetICN(0 > green ? ICN::ROUTERED : ICN::ROUTE, index);
		BlitOnTile(dst, sprite, sprite.x() - 14, sprite.y(), mp);
	    }
	}
    }

#ifdef WITH_DEBUG
    if(IS_DEVEL())
    {
	// redraw grid
	if(flag & LEVEL_ALL)
	{
	    u32 col = dst.GetColorIndex(0x40);

	    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
		for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    {
    		const Point dstpt(rectMapsPosition.x + TILEWIDTH * ox,
				rectMapsPosition.y + TILEWIDTH * oy);
		if(areaPosition & dstpt)
    		{
		    dst.Lock();
    		    dst.SetPixel(dstpt.x, dstpt.y, col);
    		    dst.Unlock();
		}

		world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawPassable(dst);
	    }
	}
    }
    else
#endif
    // redraw fog
    if(flag & LEVEL_FOG)
    {
	const u8 colors = Players::FriendColors();

	for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	    for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	{
	    const Maps::Tiles & tile = world.GetTiles(rectMaps.x + ox, rectMaps.y + oy);

	    if(tile.isFog(colors))
		tile.RedrawFogs(dst, colors);
	}
    }
}

/* scroll area */
void Interface::GameArea::Scroll(void)
{
    const Settings & conf = Settings::Get();

    if(scrollDirection & SCROLL_LEFT)
    {
	if(0 < scrollOffset.x)
	    scrollOffset.x -= scrollStepX;
	else
	if(0 < rectMaps.x)
	{
	    scrollOffset.x = conf.Editor() ? 0 : SCROLL_MAX - scrollStepX;
	    --rectMaps.x;
	}
    }
    else
    if(scrollDirection & SCROLL_RIGHT)
    {
	if(!conf.Editor() &&
	    scrollOffset.x < SCROLL_MAX * 2 - tailX)
	    scrollOffset.x += scrollStepX;
	else
	if(world.w() - rectMaps.w > rectMaps.x)
	{
	    scrollOffset.x = conf.Editor() ? 0 : SCROLL_MAX + scrollStepX - tailX;
	    ++rectMaps.x;
	}
    }

    if(scrollDirection & SCROLL_TOP)
    {
	if(0 < scrollOffset.y)
	    scrollOffset.y -= scrollStepY;
	else
	if(0 < rectMaps.y)
	{
	    scrollOffset.y = conf.Editor() ? 0 : SCROLL_MAX - scrollStepY;
	    --rectMaps.y;
	}
    }
    else
    if(scrollDirection & SCROLL_BOTTOM)
    {
	if(!conf.Editor() &&
	    scrollOffset.y < SCROLL_MAX * 2 - tailY)
	    scrollOffset.y += scrollStepY;
	else
	if(world.h() - rectMaps.h > rectMaps.y)
	{
	    scrollOffset.y = conf.Editor() ? 0 : SCROLL_MAX + scrollStepY - tailY;
	    ++rectMaps.y;
	}
    }

    rectMapsPosition.x = areaPosition.x - scrollOffset.x;
    rectMapsPosition.y = areaPosition.y - scrollOffset.y;

    scrollDirection = 0;
}

/* scroll area to center point maps */
void Interface::GameArea::SetCenter(const Point &pt)
{
    SetCenter(pt.x, pt.y);
}

void Interface::GameArea::SetCenter(s16 px, s16 py)
{
    Point pos(px - rectMaps.w / 2, py - rectMaps.h / 2);

    // our of range
    if(pos.x < 0) pos.x = 0;
    else
    if(pos.x > world.w() - rectMaps.w) pos.x = world.w() - rectMaps.w;

    if(pos.y < 0) pos.y = 0;
    else
    if(pos.y > world.h() - rectMaps.h) pos.y = world.h() - rectMaps.h;

    if(pos.x == rectMaps.x && pos.y == rectMaps.y) return;

    // possible fast scroll
    if(pos.y == rectMaps.y && 1 == (pos.x - rectMaps.x)) scrollDirection |= SCROLL_RIGHT;
    else
    if(pos.y == rectMaps.y && -1 == (pos.x - rectMaps.x)) scrollDirection |= SCROLL_LEFT;
    else
    if(pos.x == rectMaps.x && 1 == (pos.y - rectMaps.y)) scrollDirection |= SCROLL_BOTTOM;
    else
    if(pos.x == rectMaps.x && -1 == (pos.y - rectMaps.y)) scrollDirection |= SCROLL_TOP;
    else
    // diagonal
    if(-1 == (pos.y - rectMaps.y) && 1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_TOP | SCROLL_RIGHT;
    }
    else
    if(-1 == (pos.y - rectMaps.y) && -1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_TOP | SCROLL_LEFT;
    }
    else
    if(1 == (pos.y - rectMaps.y) && 1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_BOTTOM | SCROLL_RIGHT;
    }
    else
    if(1 == (pos.y - rectMaps.y) && -1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_BOTTOM | SCROLL_LEFT;
    }

    else
    {
	rectMaps.x = pos.x;
	rectMaps.y = pos.y;
	scrollDirection = 0;

	if(pos.x == 0) scrollOffset.x = 0;
	else
	if(pos.x == world.w() - rectMaps.w)
	{
	    scrollOffset.x = SCROLL_MAX * 2 - tailX;
	}
	else
	{
 	    scrollOffset.x = (rectMaps.w % 2 == 0 ?
		    SCROLL_MAX + TILEWIDTH / 2 : SCROLL_MAX) - tailX;
	}

	if(pos.y == 0) scrollOffset.y = 0;
	else
	if(pos.y == world.h() - rectMaps.h)
	{
	    scrollOffset.y = SCROLL_MAX * 2 - tailY;
	}
	else
	{
 	    scrollOffset.y = (rectMaps.h % 2 == 0 ?
		    SCROLL_MAX + TILEWIDTH / 2 : SCROLL_MAX) - tailY;
	}

	if(Settings::Get().Editor())
	{
	    scrollOffset.x = 0;
	    scrollOffset.y = 0;
	}

	rectMapsPosition.x = areaPosition.x - scrollOffset.x;
	rectMapsPosition.y = areaPosition.y - scrollOffset.y;

	if(Display::Get().w() > areaPosition.w)
	    scrollStepX = Settings::Get().ScrollSpeed();

	if(Display::Get().h() > areaPosition.h)
	    scrollStepY = Settings::Get().ScrollSpeed();
    }

    if(scrollDirection) Scroll();
}

void Interface::GameArea::GenerateUltimateArtifactAreaSurface(const s32 index, Surface & sf)
{
    if(Interface::NoGUI()) return;

    if(Maps::isValidAbsIndex(index))
    {
	sf.Set(448, 448);

	GameArea & gamearea = Get();
	const Rect origPosition(gamearea.areaPosition);
	gamearea.SetAreaPosition(0, 0, sf.w(), sf.h());

	const Rect & rectMaps = gamearea.GetRectMaps();
	const Rect & areaPosition = gamearea.GetArea();
	Point pt = Maps::GetPoint(index);

        gamearea.SetCenter(pt);
	gamearea.Redraw(sf, LEVEL_BOTTOM | LEVEL_TOP);

	// blit marker
	for(u8 ii = 0; ii < rectMaps.h; ++ii) if(index < Maps::GetIndexFromAbsPoint(rectMaps.x + rectMaps.w - 1, rectMaps.y + ii))
	{
	    pt.y = ii;
	    break;
	}
	for(u8 ii = 0; ii < rectMaps.w; ++ii) if(index == Maps::GetIndexFromAbsPoint(rectMaps.x + ii, rectMaps.y + pt.y))
	{
	    pt.x = ii;
	    break;
	}
	const Sprite & marker = AGG::GetICN(ICN::ROUTE, 0);
	const Point dst(areaPosition.x + pt.x * TILEWIDTH - gamearea.scrollOffset.x,
			    areaPosition.y + pt.y * TILEWIDTH - gamearea.scrollOffset.y);
	marker.Blit(dst.x, dst.y + 8, sf);

	Settings::Get().ExtGameEvilInterface() ? sf.GrayScale() : sf.Sepia();

	if(Settings::Get().QVGA())
	{
    	    Surface sf2;
    	    Surface::ScaleMinifyByTwo(sf2, sf);
    	    Surface::Swap(sf2, sf);
	}

	gamearea.SetAreaPosition(origPosition.x, origPosition.y, origPosition.w, origPosition.h);
    }
    else
    DEBUG(DBG_ENGINE, DBG_WARN, "artifact not found");
}

bool Interface::GameArea::NeedScroll(void) const
{
    return scrollDirection;
}

Cursor::themes_t Interface::GameArea::GetScrollCursor(void) const
{
    switch(scrollDirection)
    {
	case SCROLL_TOP:		  return Cursor::SCROLL_TOP;
	case SCROLL_BOTTOM:		  return Cursor::SCROLL_BOTTOM;
	case SCROLL_RIGHT:		  return Cursor::SCROLL_RIGHT;
	case SCROLL_LEFT:		  return Cursor::SCROLL_LEFT;
	case SCROLL_LEFT | SCROLL_TOP:	  return Cursor::SCROLL_TOPLEFT;
	case SCROLL_LEFT | SCROLL_BOTTOM: return Cursor::SCROLL_BOTTOMLEFT;
	case SCROLL_RIGHT | SCROLL_TOP:	  return Cursor::SCROLL_TOPRIGHT;
	case SCROLL_RIGHT | SCROLL_BOTTOM:return Cursor::SCROLL_BOTTOMRIGHT;

	default: break;
    }

    return Cursor::NONE;
}

void Interface::GameArea::SetScroll(scroll_t direct)
{
    switch(direct)
    {
	case SCROLL_LEFT:
	    if(0 < rectMaps.x || 0 < scrollOffset.x)
	    {
	    	scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_RIGHT:
	    if(world.w() - rectMaps.w > rectMaps.x || SCROLL_MAX * 2 > scrollOffset.x)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_TOP:
	    if(0 < rectMaps.y || 0 < scrollOffset.y)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_BOTTOM:
	    if(world.h() - rectMaps.h > rectMaps.y || SCROLL_MAX * 2 > scrollOffset.y)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	default: break;
    }

    scrollTime.Start();
}

/* convert area point to index maps */
s32 Interface::GameArea::GetIndexFromMousePoint(const Point & pt) const
{
    s32 result = (rectMaps.y + (pt.y - rectMapsPosition.y) / TILEWIDTH) * world.w() +
		    rectMaps.x + (pt.x - rectMapsPosition.x) / TILEWIDTH;
    const s32 & max = world.w() * world.h() - 1;

    return result > max || result < Maps::GetIndexFromAbsPoint(rectMaps.x, rectMaps.y) ? -1 : result;
}

void Interface::GameArea::SetUpdateCursor(void)
{
    updateCursor = true;
}

void Interface::GameArea::QueueEventProcessing(void)
{
    const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    const Point & mp = le.GetMouseCursor();

    s32 index = GetIndexFromMousePoint(mp);
    
    // out of range
    if(index < 0) return;

    const Rect tile_pos(rectMapsPosition.x + ((u16) (mp.x - rectMapsPosition.x) / TILEWIDTH) * TILEWIDTH,
	                rectMapsPosition.y + ((u16) (mp.y - rectMapsPosition.y) / TILEWIDTH) * TILEWIDTH,
	                TILEWIDTH, TILEWIDTH);

    // change cusor if need
    if(updateCursor || index != oldIndexPos)
    {
	cursor.SetThemes(Game::GetCursor(index));
	oldIndexPos = index;
	updateCursor = false;
    }

    // fixed pocket pc tap mode
    if(conf.ExtGameHideInterface() && conf.ShowControlPanel() && le.MouseCursor(Interface::ControlPanel::Get().GetArea())) return;



    if(conf.ExtPocketTapMode())
    {
	// drag&drop gamearea: scroll
	if(conf.ExtPocketDragDropScroll() && le.MousePressLeft())
	{
	    Point pt1 = le.GetMouseCursor();
	    Interface::Basic & I = Interface::Basic::Get();

    	    while(le.HandleEvents() && le.MousePressLeft())
	    {
		const Point & pt2 = le.GetMouseCursor();

		if(pt1 != pt2)
		{
		    s16 dx = pt2.x - pt1.x;
		    s16 dy = pt2.y - pt1.y;
		    s16 d2x = scrollStepX;
		    s16 d2y = scrollStepY;

		    while(1)
		    {
			if(d2x <= dx){ SetScroll(SCROLL_LEFT); dx -= d2x; }
			else
			if(-d2x >= dx){ SetScroll(SCROLL_RIGHT); dx += d2x; }

			if(d2y <= dy){ SetScroll(SCROLL_TOP); dy -= d2y; }
			else
			if(-d2y >= dy){ SetScroll(SCROLL_BOTTOM); dy += d2y; }

			if(NeedScroll())
			{
			    cursor.Hide();
			    Scroll();
			    I.SetRedraw(REDRAW_GAMEAREA);
			    I.Redraw();
			    cursor.Show();
			    display.Flip();
			}
			else break;
		    }
		}

		DELAY(10);
	    }
	}

	// fixed pocket pc: click on maps after scroll (pause: ~800 ms)
	scrollTime.Stop();
	if(800 > scrollTime.Get()) return;
    }

    if(le.MouseClickLeft(tile_pos) && Cursor::POINTER != cursor.Themes())
        Game::MouseCursorAreaClickLeft(index);
    else
    if(le.MousePressRight(tile_pos))
        Game::MouseCursorAreaPressRight(index);
}
