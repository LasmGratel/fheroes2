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
#ifndef H2BUTTON_H
#define H2BUTTON_H

#include "icn.h"
#include "gamedefs.h"

class Surface;

class Button : public Rect
{
public:
    Button();
    Button(s16, s16, ICN::icn_t, u16 index1, u16 index2);

    bool isEnable(void) const;
    bool isDisable(void) const;
    bool isPressed(void) const;
    bool isReleased(void) const;

    void Press(void);
    void Release(void);

    void SetPos(s16, s16);
    void SetPos(const Point &);
    void SetSize(u16, u16);
    void SetSprite(ICN::icn_t, u16 index1, u16);
    void SetSprite(const Surface &, const Surface &);
    void SetDisable(bool);

    void Draw(void);
    void PressDraw(void);
    void ReleaseDraw(void);

protected:
    Surface	sf1;
    Surface	sf2;

    u32		flags;
};

class ButtonSprite : public Button
{
public:
    ButtonSprite(){}

protected:
    Surface sf;
};

class ButtonGroups
{
public:
    ButtonGroups(const Rect &, u16);
    ~ButtonGroups();
    
    void Draw(void);
    u16 QueueEventProcessing(void);

    void DisableButton1(bool);
    void DisableButton2(bool);

protected:
    Button* button1;
    Button* button2;
    u16 result1;
    u16 result2;
    u16 buttons;
};

#endif
