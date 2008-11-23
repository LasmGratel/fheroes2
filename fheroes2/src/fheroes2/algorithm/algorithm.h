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
#ifndef H2ALGORITHM_H
#define H2ALGORITHM_H

#include <list>
#include "army.h"
#include "skill.h"
#include "gamedefs.h"

class Heroes;
class Castle;
namespace Route { class Step; };

namespace Algorithm
{
    u32 CalculateExperience(const Army::army_t & army);
    u32 CalculateExperience(const Heroes & hero);
    u32 CalculateExperience(const Castle &castle);

    bool PathFind(std::list<Route::Step> *result, const u16 from, const u16 to, const u16 limit = MAXU16, const Skill::Level::type_t pathfinding = Skill::Level::NONE, const u8 under = MP2::OBJ_ZERO);
};

#endif
