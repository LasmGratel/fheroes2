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
#ifndef H2DIRECTION_H
#define H2DIRECTION_H

#include <string>
#include "gamedefs.h"

namespace Direction
{
    typedef enum
    {
	UNKNOWN		= 0x0000,
	TOP_LEFT	= 0x0001,
	TOP		= 0x0002,
	TOP_RIGHT	= 0x0004,
	RIGHT		= 0x0008,
	BOTTOM_RIGHT	= 0x0010,
	BOTTOM		= 0x0020,
	BOTTOM_LEFT	= 0x0040,
	LEFT		= 0x0080,
	CENTER		= 0x0100,
    } vector_t;

    inline vector_t& operator++ (vector_t& direct){ return direct = ( CENTER == direct ? TOP_LEFT : vector_t(direct << 1)); };
    inline vector_t& operator-- (vector_t& direct){ return direct = ( TOP_LEFT == direct ? CENTER : vector_t(direct >> 1)); };

    const std::string & String(vector_t direct);

    vector_t Get(u16 from, u16 to);

    bool ShortDistanceClockWise(const vector_t from, const vector_t to);
};

#define DIRECTION_TOP_ROW	(Direction::TOP_LEFT | Direction::TOP | Direction::TOP_RIGHT)
#define DIRECTION_BOTTOM_ROW	(Direction::BOTTOM_LEFT | Direction::BOTTOM | Direction::BOTTOM_RIGHT)
#define DIRECTION_LEFT_ROW	(Direction::TOP_LEFT | Direction::LEFT | Direction::BOTTOM_LEFT)
#define DIRECTION_RIGHT_ROW	(Direction::TOP_RIGHT | Direction::RIGHT | Direction::BOTTOM_RIGHT)
#define DIRECTION_ALL		(Direction::CENTER | DIRECTION_TOP_ROW | DIRECTION_BOTTOM_ROW | DIRECTION_LEFT_ROW | DIRECTION_RIGHT_ROW)

#define DIRECTION_TOP_RIGHT_CORNER	(Direction::TOP | Direction::TOP_RIGHT | Direction::RIGHT)
#define DIRECTION_TOP_LEFT_CORNER	(Direction::TOP | Direction::TOP_LEFT | Direction::LEFT)
#define DIRECTION_BOTTOM_RIGHT_CORNER	(Direction::BOTTOM | Direction::BOTTOM_RIGHT | Direction::RIGHT)
#define DIRECTION_BOTTOM_LEFT_CORNER	(Direction::BOTTOM | Direction::BOTTOM_LEFT | Direction::LEFT)

#endif
