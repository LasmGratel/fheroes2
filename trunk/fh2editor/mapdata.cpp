/***************************************************************************
 *   Copyright (C) 2013 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include <algorithm>
#include <QtGui>
#include <QPainter>
#include <QDomDocument>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

#include "engine.h"
#include "global.h"
#include "program.h"
#include "dialogs.h"
#include "mainwindow.h"
#include "mapwindow.h"
#include "mapdata.h"

MapTileExt::MapTileExt(int lvl, const mp2lev_t & ext)
    : spriteICN(H2::mapICN(ext.object)), spriteExt(ext.object), spriteIndex(ext.index), spriteLevel(lvl), spriteUID(ext.uniq)
{
}

MapTileExt::MapTileExt(const CompositeObject & co, const CompositeSprite & cs, quint32 uid)
    : spriteICN(co.icn.second), spriteExt(0), spriteIndex(cs.spriteIndex), spriteLevel(0), spriteUID(uid)
{
    if(cs.spriteAnimation)
	spriteExt |= 0x01;
}

bool MapTileExt::isAnimation(const MapTileExt & mte)
{
    return mte.spriteExt & 0x01;
}

bool MapTileExt::sortLevel1(const MapTileExt & mte1, const MapTileExt & mte2)
{
    return (mte1.spriteLevel % 4) > (mte2.spriteLevel % 4);
}

bool MapTileExt::sortLevel2(const MapTileExt & mte1, const MapTileExt & mte2)
{
    return (mte1.spriteLevel % 4) < (mte2.spriteLevel % 4);
}

bool MapTileExt::isTown(const MapTileExt & te)
{
    return ICN::OBJNTOWN == te.spriteICN ? true : isRandomTown(te);
}

bool MapTileExt::isRandomTown(const MapTileExt & te)
{
    return ICN::OBJNTWRD == te.spriteICN && 32 > te.spriteIndex;
}

bool MapTileExt::isMiniHero(const MapTileExt & te)
{
    return ICN::MINIHERO == te.spriteICN;
}

bool MapTileExt::isSign(const MapTileExt & te)
{
    return (ICN::OBJNMUL2 == te.spriteICN && 114 == te.spriteIndex) ||
	    (ICN::OBJNDSRT == te.spriteICN && 119 == te.spriteIndex) ||
	    (ICN::OBJNLAVA == te.spriteICN && 117 == te.spriteIndex) ||
	    (ICN::OBJNSNOW == te.spriteICN && 143 == te.spriteIndex) ||
	    (ICN::OBJNSWMP == te.spriteICN && 140 == te.spriteIndex);
}

bool MapTileExt::isResource(const MapTileExt & te)
{
    return Resource::Unknown != resource(te);
}

int MapTileExt::resource(const MapTileExt & te)
{
    if(ICN::OBJNRSRC == te.spriteICN)
    {
	switch(te.spriteIndex)
	{
	    case 1:	return Resource::Wood;
	    case 3:	return Resource::Mercury;
	    case 5:	return Resource::Ore;
	    case 7:	return Resource::Sulfur;
	    case 9:	return Resource::Crystal;
	    case 11:	return Resource::Gems;
	    case 13:	return Resource::Gold;
	    case 17:	return Resource::Random;
	    default: break;
	}
    }

    return Resource::Unknown;
}

bool MapTileExt::isButtle(const MapTileExt & te)
{
    return ICN::OBJNWATR == te.spriteICN && 0 == te.spriteIndex;
}

bool MapTileExt::isSphinx(const MapTileExt & te)
{
    return ICN::OBJNDSRT == te.spriteICN && (85 <= te.spriteIndex || 88 <= te.spriteIndex);
}

bool MapTileExt::isMapEvent(const MapTileExt & te)
{
    return ICN::OBJNMUL2 == te.spriteICN && 163 == te.spriteIndex;
}

int MapTileExt::loyaltyObject(const MapTileExt & te)
{
    switch(te.spriteICN)
    {
        case ICN::X_LOC1:
            if(te.spriteIndex == 3) return MapObj::AlchemyTower | MapObj::IsAction;
            else
            if(te.spriteIndex < 3) return MapObj::AlchemyTower;
            else
            if(70 == te.spriteIndex) return MapObj::Arena | MapObj::IsAction;
            else
            if(3 < te.spriteIndex && te.spriteIndex < 72) return MapObj::Arena;
            else
            if(77 == te.spriteIndex) return MapObj::BarrowMounds | MapObj::IsAction;
            else
            if(71 < te.spriteIndex && te.spriteIndex < 78) return MapObj::BarrowMounds;
            else
            if(94 == te.spriteIndex) return MapObj::EarthAltar | MapObj::IsAction;
            else
            if(77 < te.spriteIndex && te.spriteIndex < 112) return MapObj::EarthAltar;
            else
            if(118 == te.spriteIndex) return MapObj::AirAltar | MapObj::IsAction;
            else
            if(111 < te.spriteIndex && te.spriteIndex < 120) return MapObj::AirAltar;
            else
            if(127 == te.spriteIndex) return MapObj::FireAltar | MapObj::IsAction;
            else
            if(119 < te.spriteIndex && te.spriteIndex < 129) return MapObj::FireAltar;
            else
            if(135 == te.spriteIndex) return MapObj::WaterAltar | MapObj::IsAction;
            else
            if(128 < te.spriteIndex && te.spriteIndex < 137) return MapObj::WaterAltar;
            break;

        case ICN::X_LOC2:
            if(te.spriteIndex == 4) return MapObj::Stables | MapObj::IsAction;
            else
            if(te.spriteIndex < 4) return MapObj::Stables;
            else
            if(te.spriteIndex == 9) return MapObj::Jail | MapObj::IsAction;
            else
            if(4 < te.spriteIndex && te.spriteIndex < 10) return MapObj::Jail;
            else
            if(te.spriteIndex == 37) return MapObj::Mermaid | MapObj::IsAction;
            else
            if(9 < te.spriteIndex && te.spriteIndex < 47) return MapObj::Mermaid;
            else
            if(te.spriteIndex == 101) return MapObj::Sirens | MapObj::IsAction;
            else
            if(46 < te.spriteIndex && te.spriteIndex < 111) return MapObj::Sirens;
            else
            if(110 < te.spriteIndex && te.spriteIndex < 136) return MapObj::Reefs;
            break;

        case ICN::X_LOC3:
            if(te.spriteIndex == 30) return MapObj::HutMagi | MapObj::IsAction;
            else
            if(te.spriteIndex < 32) return MapObj::HutMagi;
            else
            if(te.spriteIndex == 50) return MapObj::EyeMagi | MapObj::IsAction;
            else
            if(31 < te.spriteIndex && te.spriteIndex < 59) return MapObj::EyeMagi;
            break;

        default: break;
    }

    return MapObj::None;
}

QDomElement & operator<< (QDomElement & el, const MapTileExt & ext)
{
    el.setAttribute("icn", ext.spriteICN);
    el.setAttribute("ext", ext.spriteExt);
    el.setAttribute("index", ext.spriteIndex);
    el.setAttribute("level", ext.spriteLevel);
    el.setAttribute("uid", ext.spriteUID);

    return el;
}

QDomElement & operator>> (QDomElement & el, MapTileExt & ext)
{
    ext.spriteICN = el.attribute("icn").toInt();
    ext.spriteExt = el.attribute("ext").toInt();
    ext.spriteIndex = el.attribute("index").toInt();
    ext.spriteLevel = el.attribute("level").toInt();
    ext.spriteUID = el.attribute("uid").toInt();

    return el;
}

void MapTileLevels::paint(QPainter & painter, const QPoint & offset, const QPoint & mpos) const
{
    for(const_iterator it = begin(); it != end(); ++it)
    {
	QPair<QPixmap, QPoint> p1 = EditorTheme::getImageICN((*it).icn(), (*it).index());
	painter.drawPixmap(offset + p1.second, p1.first);

	if(MapTileExt::isAnimation(*it))
	{
	    int anim = H2::isAnimationICN((*it).icn(), (*it).index(), 0);

	    if(0 < anim)
	    {
		QPair<QPixmap, QPoint> p2 = EditorTheme::getImageICN((*it).icn(), anim);
		painter.drawPixmap(offset + p2.second, p2.first);
	    }
	    else
		qDebug() << "H2::isAnimationICN:" << "incorrect animation" << mpos << "icn:" << (*it).icn() << "index:" << (*it).index();
	}
    }
}

QString MapTileLevels::infoString(void) const
{
    QString str;
    QTextStream ss(& str);

    for(const_iterator it = begin(); it != end(); ++it)
    {
	ss <<
	    "uniq:   " << (*it).uid() << endl <<
	    "sprite: " << H2::icnString((*it).icn()) << ", " <<  (*it).index() << endl <<
	    "level:  " << (*it).level() << endl;
    }

    return str;
}

const MapTileExt* MapTileLevels::find(bool (*pf)(const MapTileExt &)) const
{
    const_iterator it = begin();
    for(; it != end(); ++it) if(pf(*it)) break;
    return it != end() ? &(*it) : NULL;
}

int MapTileLevels::topObjectID(void) const
{
    int id = MapObj::None;

    for(const_iterator it = begin(); it != end() && id == MapObj::None; ++it)
	id = EditorTheme::getObjectID((*it).icn(), (*it).index());

    return id;
}

bool MapTileLevels::removeSprite(quint32 uid)
{
    return removeAll(MapTileExt(uid));
}

QDomElement & operator<< (QDomElement & el, const MapTileLevels & levels)
{
    for(MapTileLevels::const_iterator
	it = levels.begin(); it != levels.end(); ++it)
    {
	QDomElement sprite = el.ownerDocument().createElement("sprite");
	el.appendChild(sprite);
	sprite << *it;
    }

    return el;
}

QDomElement & operator>> (QDomElement & el, MapTileLevels & levels)
{
    levels.clear();
    QDomNodeList list = el.elementsByTagName("sprite");

    for(int pos = 0; pos < list.size(); ++pos)
    {
	levels << MapTileExt();    
	QDomElement sprite = list.item(pos).toElement();
	sprite >> levels.back();
    }

    return el;
}

MapTile::MapTile(const mp2til_t & mp2, const QPoint & pos)
    : mpos(pos), tileSprite(mp2.tileSprite), tileShape(mp2.tileShape % 4), objectID(mp2.objectID),
	passableBase(Direction::All), passableLocal(Direction::All)
{
    setGraphicsPixmapItemValues();
    loadSpriteLevels(mp2.ext);

    // fix MP2 objects
    switch(mp2.objectID)
    {
	// Shrub2
        case 0x38:      objectID = MapObj::Shrub; break;
	// Nothing Special
        case 0x39:      objectID = MapObj::None; break;
        case 0xE9:      objectID = MapObj::Reefs; break;
	// change treasurechest to waterchest
        case 0x86:      objectID = (EditorTheme::ground(tileSprite) != Ground::Water ? MapObj::TreasureChest : MapObj::WaterChest) | MapObj::IsAction; break;
	// fix loyalty obj
        case 0x79:
        case 0x7A:
        case 0xF9:
        case 0xFA:	objectID = MapObj::None; break;

        default: break;
    }

    if(objectID == MapObj::None)
    {
	for(MapTileLevels::const_iterator
	    it = spritesLevel1.begin(); it != spritesLevel1.end() && objectID == MapObj::None; ++it)
	    objectID = MapTileExt::loyaltyObject(*it);

	if(objectID == MapObj::None)
	{
	    for(MapTileLevels::const_iterator
		it = spritesLevel2.begin(); it != spritesLevel2.end() && objectID == MapObj::None; ++it)
		objectID = MapTileExt::loyaltyObject(*it);
	}
    }
}

MapTile::MapTile(const MapTile & other)
    : QGraphicsPixmapItem(), mpos(other.mpos), tileSprite(other.tileSprite), tileShape(other.tileShape), objectID(other.objectID),
	spritesLevel1(other.spritesLevel1), spritesLevel2(other.spritesLevel2), passableBase(other.passableBase), passableLocal(other.passableLocal)
{
    setGraphicsPixmapItemValues();
}

MapTile::MapTile()
    : QGraphicsPixmapItem(), tileSprite(0), tileShape(0), objectID(MapObj::None), passableBase(Direction::All), passableLocal(Direction::All)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

MapTile & MapTile::operator=(const MapTile & other)
{
    mpos = other.mpos;
    tileSprite = other.tileSprite;
    tileShape = other.tileShape;
    objectID = other.objectID;

    spritesLevel1 = other.spritesLevel1;
    spritesLevel2 = other.spritesLevel2;

    passableBase = other.passableBase;
    passableLocal = other.passableLocal;

    setGraphicsPixmapItemValues();

    return *this;
}

void MapTile::setGraphicsPixmapItemValues(void)
{
    const QSize & tileSize = EditorTheme::tileSize();
    setOffset(mpos.x() * tileSize.width(), mpos.y() * tileSize.height());
    setFlags(QGraphicsItem::ItemIsSelectable);
    setTileSprite(tileSprite, tileShape);
}

void MapTile::setTileSprite(int index, int rotate)
{
    tileSprite = index;
    tileShape = rotate % 4;

    QPixmap sprite = EditorTheme::getImageTIL("GROUND32.TIL", tileSprite);

    switch(tileShape)
    {
	case 1: setPixmap(sprite.transformed(QTransform().scale( 1, -1))); break;
	case 2: setPixmap(sprite.transformed(QTransform().scale(-1,  1))); break;
	case 3: setPixmap(sprite.transformed(QTransform().scale(-1, -1))); break;
	default: setPixmap(sprite); break;
    }
}

QRectF MapTile::boundingRect(void) const
{
    return QRectF(offset(), pixmap().size());
}

int MapTile::groundType(void) const
{
    return EditorTheme::ground(tileSprite);
}

QString MapTile::indexString(int index)
{
    QString str;

    QTextStream out(& str);
    out.setFieldWidth(3);
    out.setPadChar('0');

    out << index;

    return str;
}

void MapTile::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    // draw tile
    painter->drawPixmap(offset(), pixmap());

    // draw level1
    spritesLevel1.paint(*painter, offset().toPoint(), mpos);

    // draw level2
    spritesLevel2.paint(*painter, offset().toPoint(), mpos);
}

void MapTile::showInfo(void) const
{
    QString msg;
    QTextStream ss2(& msg);

    ss2 << "tile pos:    " << mpos.x() << ", " << mpos.y() << endl \
	<< "tile sprite: " << tileSprite << endl \
	<< "tile rotate: " << tileShape << endl \
	<< "object:      " << MapObj::transcribe(objectID) << endl;

    ss2 << "----------------------" << endl;

    // draw level2
    if(spritesLevel1.size())
	ss2 << spritesLevel1.infoString() <<
	    "----------------------" << endl;

    // draw level2
    if(spritesLevel2.size())
	ss2 << spritesLevel2.infoString() <<
	    "----------------------" << endl;

    QMessageBox::information(NULL, "Tile Info", msg);
}

void MapTile::loadSpriteLevel(MapTileLevels & list, int level, const mp2lev_t & ext)
{
    if(ext.object && ext.index < 0xFF)
    {
	const QString & icn = H2::icnString(H2::mapICN(ext.object));

	if(! icn.isEmpty())
	    list << MapTileExt(level, ext);
    }
}

bool MapTile::isAction(void) const
{
    return objectID & MapObj::IsAction;
}

int MapTile::object(void) const
{
    return objectID & ~MapObj::IsAction;
}

void MapTile::loadSpriteLevels(const mp2ext_t & mp2)
{
    // level1
    loadSpriteLevel(spritesLevel1, mp2.quantity, mp2.level1);

    // level2
    loadSpriteLevel(spritesLevel2, mp2.quantity, mp2.level2);
}

void MapTile::sortSpritesLevels(void)
{
    qStableSort(spritesLevel1.begin(), spritesLevel1.end(), MapTileExt::sortLevel1);
    qStableSort(spritesLevel2.begin(), spritesLevel2.end(), MapTileExt::sortLevel2);
}

void MapTile::addSpriteSection(const CompositeObject & co, const CompositeSprite & cs, quint32 uid)
{
    if(cs.spriteLevel == SpriteLevel::Top)
	spritesLevel2 << MapTileExt(co, cs, uid);
    else
	spritesLevel1 << MapTileExt(co, cs, uid);

    if(spritesLevel2.size())
	objectID = spritesLevel2.topObjectID();

    if(objectID == MapObj::None && spritesLevel1.size())
	objectID = spritesLevel1.topObjectID();
}

void MapTile::removeSpriteSection(quint32 uid)
{
    spritesLevel1.removeSprite(uid);
    spritesLevel2.removeSprite(uid);
}

QDomElement & operator<< (QDomElement & el, const MapTile & tile)
{
    el << tile.mpos;

    el.setAttribute("base", tile.passableBase);
    el.setAttribute("local", tile.passableLocal);

    el.setAttribute("tileSprite", tile.tileSprite);
    el.setAttribute("tileShape", tile.tileShape);
    el.setAttribute("objectID", tile.objectID);

    QDomElement el1 = el.ownerDocument().createElement("levels1");
    el.appendChild(el1);
    el1 << tile.spritesLevel1;

    QDomElement el2 = el.ownerDocument().createElement("levels2");
    el.appendChild(el2);
    el1 << tile.spritesLevel2;

    return el;
}

QDomElement & operator>> (QDomElement & el, MapTile & tile)
{
    el >> tile.mpos;

    tile.passableBase = el.hasAttribute("base") ? el.attribute("base").toInt() : Direction::All;
    tile.passableLocal = el.hasAttribute("local") ? el.attribute("local").toInt() : Direction::All;

    tile.tileSprite = el.attribute("tileSprite").toInt();
    tile.tileShape = el.attribute("tileShape").toInt();
    tile.objectID = el.attribute("objectID").toInt();

    QDomElement el1 = el.firstChildElement("levels1");
    el1 >> tile.spritesLevel1;

    QDomElement el2 = el.firstChildElement("levels2");
    el2 >> tile.spritesLevel2;

    tile.setGraphicsPixmapItemValues();

    return el;
}

MapTiles::MapTiles(const MapTiles & tiles, const QRect & area) : msize(area.size())
{
    reserve(msize.width() * msize.height());

    for(int yy = area.y(); yy < area.y() + area.height(); ++yy)
    {
	for(int xx = area.x(); xx < area.x() + area.width(); ++xx)
        {
	    const MapTile* tile = tiles.tileConst(QPoint(xx, yy));
	    if(tile) *this << MapTile(*tile);
        }
    }
}

QString MapTiles::sizeDescription(void) const
{
    if(msize.width() == msize.height())
    {
	switch(msize.width())
	{
	    case 36:	return "small";
	    case 72:	return "medium";
	    case 108:	return "large";
	    case 144:	return "extra large";
	    default: break;
	}
    }

    return "custom";
}

void MapTiles::newMap(const QSize & sz)
{
    msize = sz;
    reserve(msize.width() * msize.height());

    for(int yy = 0; yy < msize.height(); ++yy)
    {
	for(int xx = 0; xx < msize.width(); ++xx)
    	    *this << MapTile(mp2til_t(), QPoint(xx, yy));
    }
}

bool MapTiles::importMap(const QSize & sz, const QVector<mp2til_t> & mp2Tiles, const QVector<mp2ext_t> & mp2Sprites)
{
    msize = sz;
    reserve(msize.width() * msize.height());

    for(int yy = 0; yy < msize.height(); ++yy)
    {
	for(int xx = 0; xx < msize.width(); ++xx)
	{
	    const mp2til_t & mp2til = mp2Tiles[indexPoint(QPoint(xx, yy))];
	    *this << MapTile(mp2til, QPoint(xx, yy));
	    int ext = mp2til.ext.indexExt;

	    while(ext)
	    {
		if(ext >= mp2Sprites.size())
		{
		    qDebug() << "ext block: out of range" << ext;
    		    return false;
		}

		back().loadSpriteLevels(mp2Sprites[ext]);
		ext = mp2Sprites[ext].indexExt;
	    }

	    back().sortSpritesLevels();
	}
    }

    return true;
}

const MapTile* MapTiles::mapToTileConst(const QPoint & pos) const
{
    for(MapTiles::const_iterator
	it = begin(); it != end(); ++it)
	if((*it).boundingRect().contains(pos)) return &(*it);

    return NULL;
}

MapTile* MapTiles::mapToTile(const QPoint & pos)
{
    return const_cast<MapTile*>(mapToTileConst(pos));
}

const MapTile* MapTiles::tileConst(const QPoint & pos) const
{
    const QVector<MapTile> & mapTiles = *this;
    return isValidPoint(pos) ? & mapTiles[indexPoint(pos)] : NULL;
}

MapTile* MapTiles::tile(const QPoint & pos)
{
    return const_cast<MapTile*>(tileConst(pos));
}

const MapTile* MapTiles::tileFromDirectionConst(const MapTile* tile, int direct) const
{
    return tile ? tileFromDirectionConst(tile->mapPos(), direct) : NULL;
}

MapTile* MapTiles::tileFromDirection(const MapTile* tile, int direct)
{
    return const_cast<MapTile*>(tileFromDirectionConst(tile, direct));
}

const MapTile* MapTiles::tileFromDirectionConst(const QPoint & center, int direct) const
{
    QPoint next(center);

    switch(direct)
    {
	case Direction::Top:         if(center.y()) next.setY(center.y() - 1); break;
    	case Direction::Bottom:      if(center.y() < msize.height()) next.setY(center.y() + 1); break;
    	case Direction::Left:        if(center.x()) next.setX(center.x() - 1); break;
    	case Direction::Right:       if(center.x() < msize.width()) next.setX(center.x() + 1); break;

    	case Direction::TopRight:    return tileFromDirectionConst(tileFromDirectionConst(center, Direction::Top), Direction::Right);
    	case Direction::BottomRight: return tileFromDirectionConst(tileFromDirectionConst(center, Direction::Bottom), Direction::Right);
    	case Direction::BottomLeft:  return tileFromDirectionConst(tileFromDirectionConst(center, Direction::Bottom), Direction::Left);
    	case Direction::TopLeft:     return tileFromDirectionConst(tileFromDirectionConst(center, Direction::Top), Direction::Left);
    	default: break;
    }

    return tileConst(next);
}

MapTile* MapTiles::tileFromDirection(const QPoint & center, int direct)
{
    return const_cast<MapTile*>(tileFromDirectionConst(center, direct));
}

void MapTiles::removeSprites(quint32 uid)
{
    for(iterator it = begin(); it != end(); ++it)
	(*it).removeSpriteSection(uid);
}

int MapTiles::indexPoint(const QPoint & pos) const
{
    return pos.x() + pos.y() * msize.width();
}

bool MapTiles::isValidPoint(const QPoint & pos) const
{
    return QRect(QPoint(0, 0), msize).contains(pos);
}

void MapTiles::insertToScene(QGraphicsScene & scene)
{
    for(iterator it = begin(); it != end(); ++it)
	scene.addItem(& (*it));
}

void MapTiles::fixedOffset(void)
{
    if(size())
    {
	QPoint offset = front().mapPos();

	if(offset != QPoint(0, 0))
	    for(iterator it = begin(); it != end(); ++it)
		(*it).setMapPos((*it).mapPos() - offset);
    }
}

QDomElement & operator<< (QDomElement & el, const MapTiles & tiles)
{
    el << tiles.msize;

    for(MapTiles::const_iterator
	it = tiles.begin(); it != tiles.end(); ++it)
    {
	QDomElement elem = el.ownerDocument().createElement("tile");
	el.appendChild(elem);
	elem << *it;
    }

    return el;
}

QDomElement & operator>> (QDomElement & el, MapTiles & tiles)
{
    el >> tiles.msize;

    tiles.clear();
    QDomNodeList nodeList = el.elementsByTagName("tile");

    if(tiles.msize.width() * tiles.msize.height() != nodeList.size())
	qCritical() << "read tiles: " << "incorrect array";

    for(int pos = 0; pos < nodeList.size(); ++pos)
    {
	QDomElement elem = nodeList.item(pos).toElement();
	tiles << MapTile();
	elem >> tiles.back();
    }

    return el;
}

MapArea::MapArea(const MapArea & ma, const QRect & rt) : tiles(ma.tiles, rt), objects(ma.objects, rt), uniq(ma.uniq)
{
    tiles.fixedOffset();
}

void MapArea::importMP2Towns(const QVector<H2::TownPos> & towns)
{
    for(QVector<H2::TownPos>::const_iterator
	it = towns.begin(); it != towns.end(); ++it) if(tiles.isValidPoint((*it).pos()))
    {
	const MapTileExt* ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isTown);
	int uid = ext ? ext->uid() : -1;
	objects.push_back(new MapTown((*it).pos(), uid, (*it).town()));
    }
}

void MapArea::importMP2Heroes(const QVector<H2::HeroPos> & heroes)
{
    for(QVector<H2::HeroPos>::const_iterator
	it = heroes.begin(); it != heroes.end(); ++it) if(tiles.isValidPoint((*it).pos()))
    {
	const MapTileExt* ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isMiniHero);
	int uid = ext ? ext->uid() : -1;
	objects.push_back(new MapHero((*it).pos(), uid, (*it).hero()));
    }
}

void MapArea::importMP2Signs(const QVector<H2::SignPos> & signs)
{
    for(QVector<H2::SignPos>::const_iterator
	it = signs.begin(); it != signs.end(); ++it) if(tiles.isValidPoint((*it).pos()))
    {
	const MapTileExt* ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isSign);
	if(!ext) ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isButtle);
	int uid = ext ? ext->uid() : -1;
	objects.push_back(new MapSign((*it).pos(), uid, (*it).sign()));
    }
}

void MapArea::importMP2MapEvents(const QVector<H2::EventPos> & events)
{
    for(QVector<H2::EventPos>::const_iterator
	it = events.begin(); it != events.end(); ++it) if(tiles.isValidPoint((*it).pos()))
    {
	const MapTileExt* ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isMapEvent);
	int uid = ext ? ext->uid() : -1;
	objects.push_back(new MapEvent((*it).pos(), uid, (*it).event()));
    }
}

void MapArea::importMP2SphinxRiddles(const QVector<H2::SphinxPos> & sphinxes)
{
    for(QVector<H2::SphinxPos>::const_iterator
	it = sphinxes.begin(); it != sphinxes.end(); ++it) if(tiles.isValidPoint((*it).pos()))
    {
	const MapTileExt* ext = tiles.tileConst((*it).pos())->levels1().find(MapTileExt::isSphinx);
	int uid = ext ? ext->uid() : -1;
	objects.push_back(new MapSphinx((*it).pos(), uid, (*it).sphinx()));
    }
}

void MapArea::importArea(const MapArea & area, const QPoint & dst)
{
    QSize mapSize = area.tiles.mapSize();

    if(dst.x() + area.tiles.mapSize().width() > tiles.mapSize().width())
	mapSize.setWidth(tiles.mapSize().width() - dst.x() + area.tiles.mapSize().width());

    if(dst.y() + area.tiles.mapSize().height() > tiles.mapSize().height())
	mapSize.setHeight(tiles.mapSize().height() - dst.y() + area.tiles.mapSize().height());

    for(int yy = 0; yy < mapSize.height(); ++yy)
    {
	for(int xx = 0; xx < mapSize.width(); ++xx)
	{
	    const MapTile* srcTile = area.tiles.tileConst(QPoint(xx, yy));
	    MapTile* dstTile = tiles.tile(QPoint(dst.x() + xx, dst.y() + yy));

	    if(srcTile && dstTile)
	    {
		QPoint tmp = dstTile->mapPos();
		*dstTile = *srcTile;
		dstTile->setMapPos(tmp);
	    }
	}
    }
}

QDomElement & operator<< (QDomElement & el, const MapArea & area)
{
    QDomElement eobjects = el.ownerDocument().createElement("objects");
    eobjects.setAttribute("lastUID", area.uniq);
    el.appendChild(eobjects);
    eobjects << area.objects;

    QDomElement etiles = el.ownerDocument().createElement("tiles");
    el.appendChild(etiles);
    etiles << area.tiles;

    return el;
}

QDomElement & operator>> (QDomElement & el, MapArea & area)
{
    QDomElement eobjects = el.firstChildElement("objects");
    eobjects >> area.objects;
    area.uniq = eobjects.hasAttribute("lastUID") ? eobjects.attribute("lastUID").toInt() : 0;

    QDomElement etiles = el.firstChildElement("tiles");
    etiles >> area.tiles;

    return el;
}

QSharedPointer<MapArea> MapData::selectedArea = QSharedPointer<MapArea>();

MapData::MapData(MapWindow* parent) : QGraphicsScene(parent), tileOverMouse(NULL),
    mapName("New Map"), mapAuthors("unknown"), mapLicense("unknown"), mapDifficulty(Difficulty::Normal),
    mapKingdomColors(0), mapCompColors(0), mapHumanColors(0), mapStartWithHero(false), mapArea(),
    mapTiles(mapArea.tiles), mapObjects(mapArea.objects), engineVersion(FH2ENGINE_CURRENT_VERSION), mapVersion(engineVersion)
{
    connect(this, SIGNAL(dataModified()), this, SLOT(generateMiniMap()));
}

const QString & MapData::name(void) const
{
    return mapName;
}

const QString & MapData::description(void) const
{
    return mapDescription;
}

const QString & MapData::authors(void) const
{
    return mapAuthors;
}

const QString & MapData::license(void) const
{
    return mapLicense;
}

int MapData::difficulty(void) const
{
    return mapDifficulty;
}

int MapData::kingdomColors(void) const
{
    return mapKingdomColors;
}

int MapData::humanColors(void) const
{
    return mapHumanColors;
}

int MapData::computerColors(void) const
{
    return mapCompColors;
}

bool MapData::startWithHero(void) const
{
    return mapStartWithHero;
}

const CondWins & MapData::conditionWins(void) const
{
    return mapConditionWins;
}

const CondLoss & MapData::conditionLoss(void) const
{
    return mapConditionLoss;
}

ListStringPos MapData::conditionHeroList(int cond) const
{
    Q_UNUSED(cond);

    ListStringPos res;
    QList<SharedMapObject> listHeroes = mapObjects.list(MapObj::Heroes);

    for(QList<SharedMapObject>::const_iterator
	it = listHeroes.begin(); it != listHeroes.end(); ++it)
        res << QPair<QString, QPoint>((*it).data()->name(), (*it).data()->pos());

    return res;
}

ListStringPos MapData::conditionTownList(int cond) const
{
    Q_UNUSED(cond);

    ListStringPos res;
    QList<SharedMapObject> listCastles = mapObjects.list(MapObj::Castle);

    for(QList<SharedMapObject>::const_iterator
	it = listCastles.begin(); it != listCastles.end(); ++it)
        res << QPair<QString, QPoint>((*it).data()->name(), (*it).data()->pos());

    return res;
}

ListStringPos MapData::conditionArtifactList(void) const
{
    ListStringPos res;

    res << QPair<QString, QPoint>("Test 1", QPoint(62,83)) << QPair<QString, QPoint>("Artifact 2", QPoint(81,27));

    return res;
}

QList<QString> MapData::conditionSideList(void) const
{
    return QList<QString>() << "Left vs Right" << "Right vs Left";
}

const QSize & MapData::size(void) const
{
    return mapTiles.mapSize();
}

quint32 MapData::uniq(void)
{
    return mapArea.uniq++;
}

const QStringList & MapData::tavernRumorsList(void) const
{
    return tavernRumors;
}

void MapData::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // clear selected
    if(selectedItems().size())
    {
	if((event->buttons() & Qt::LeftButton) ||
	    ((event->buttons() & Qt::RightButton) && ! selectionArea().contains(event->scenePos())))
	{
	    clearSelection();
	    selectedArea.clear();
	}
    }
    else
    // click action object
    if(tileOverMouse && tileOverMouse->isAction())
    {
	if(event->buttons() & Qt::LeftButton)
	    emit clickActionObject(tileOverMouse);
    }
    else
    // place object
    if(currentObject.isValid())
    {
	if(event->buttons() & Qt::LeftButton)
	    addMapObject(currentObject.scenePos, currentObject, uniq());
	else
	    currentObject.reset();
	update(currentObject.area());
    }

    event->accept();
}

void MapData::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if(currentObject.isValid())
    {
	update(currentObject.area());
    }
    else
    // select area
    if(event->buttons() & Qt::LeftButton)
    {
	if(selectedItems().size())
	    clearSelection();

	selectArea(event->buttonDownScenePos(Qt::LeftButton), event->scenePos());
    }

    // update select item over cursor
    if(tileOverMouse)
	update(tileOverMouse->boundingRect());

    MapTile* newTileOverMouse = qgraphicsitem_cast<MapTile*>(itemAt(event->scenePos()));

    if(newTileOverMouse)
    {
	if(tileOverMouse != newTileOverMouse)
	{
	    MapWindow* mapWindow = qobject_cast<MapWindow*>(parent());

	    if(!tileOverMouse || tileOverMouse->mapPos().x() != newTileOverMouse->mapPos().x())
		    emit mapWindow->cursorTileXPosChanged(newTileOverMouse->mapPos().x());

	    if(!tileOverMouse || tileOverMouse->mapPos().y() != newTileOverMouse->mapPos().y())
		    emit mapWindow->cursorTileYPosChanged(newTileOverMouse->mapPos().y());

	    tileOverMouse = newTileOverMouse;
	}

	if(tileOverMouse)
	    update(tileOverMouse->boundingRect());
    }

    event->accept();
}

void MapData::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if(tileOverMouse)
    {
	if(tileOverMouse->isAction())
	{
	    editObjectAttributes();
	}
	else
	{
	    cellInfoDialog();
	}
    }

    event->accept();
}

void MapData::selectArea(QPointF ptdn, QPointF ptup)
{
    if(ptup != ptdn)
    {
	if(ptup.x() < ptdn.x())
	    qSwap(ptup.rx(), ptdn.rx());

	if(ptup.y() < ptdn.y())
	    qSwap(ptup.ry(), ptdn.ry());

	QRect selRect = QRectF(ptdn, ptup).toRect();

	const QSize & tileSize = EditorTheme::tileSize();

	int sl = selRect.left() / tileSize.width();
	int st = selRect.top() / tileSize.height();
	int sr = selRect.right() / tileSize.width();
	int sb = selRect.bottom() / tileSize.height();

	if(selRect.left() > sl * tileSize.width())
	    selRect.setLeft(sl * tileSize.width());

	if(selRect.top() > st * tileSize.height())
	    selRect.setTop(st * tileSize.height());

	if(selRect.right() > sr * tileSize.width())
	    selRect.setRight((sr + 1) * tileSize.width() - 1);

	if(selRect.bottom() > sb * tileSize.height())
	    selRect.setBottom((sb + 1) * tileSize.height() - 1);

	QPainterPath path;
	path.addRect(selRect);

	setSelectionArea(path);
    }
}

void MapData::selectAllTiles(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const QSize & sz = EditorTheme::tileSize();
    selectArea(QPointF(0, 0), QPointF(sz.width() * size().width(), sz.height() * size().height()));
    QApplication::restoreOverrideCursor();
}

void MapData::drawForeground(QPainter* painter, const QRectF & rect)
{
    Q_UNUSED(rect);

    // paint: selected area
    if(selectedItems().size())
    {
	painter->setPen(QPen(QColor(40, 40, 100), 1));
	painter->setBrush(QBrush(QColor(40, 40, 100, 150), Qt::Dense4Pattern));
	painter->drawRoundedRect(selectionArea().boundingRect(), 6.0, 6.0);
    }
    else
    if(tileOverMouse)
    {
	// paint: selected new object place
	if(currentObject.isValid())
	{
	    QPoint pos = tileOverMouse->boundingRect().topLeft().toPoint() - currentObject.center();
	    currentObject.paint(*painter, pos, true);
	}
	else
	// paint: selected item over mouse
	{
	    painter->setPen(QPen(tileOverMouse->isAction() ? QColor(0, 255, 0) : QColor(255, 255, 0), 1));
	    painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
	    const QRectF & rt = tileOverMouse->boundingRect();
	    painter->drawRect(QRectF(rt.x(), rt.y(), rt.width() - 1, rt.height() - 1));
	}
    }
}

void MapData::editPassableDialog(void)
{
    qDebug() << "edit passable dialog";
}

void MapData::cellInfoDialog(void)
{
    if(tileOverMouse)
	tileOverMouse->showInfo();
}

void MapData::copyToBuffer(void)
{
    QList<QGraphicsItem*> selected = selectedItems();

    if(selected.size())
    {
	selectedArea = QSharedPointer<MapArea>(new MapArea(mapArea, mapToTile(selectionArea().boundingRect().toRect())));
        emit validBuffer(true);
    }
}

void MapData::pasteFromBuffer(void)
{
    if(tileOverMouse && !selectedArea.isNull())
    {
	mapArea.importArea(*selectedArea.data(), tileOverMouse->mapPos());
	emit dataModified();
    }
}

void MapData::fillGroundAction(QAction* act)
{
    if(act)
    {
	int ground = act->data().toInt();
	QList<QGraphicsItem*> selected = selectedItems();

	// fill default
	for(QList<QGraphicsItem*>::iterator
	    it = selected.begin(); it != selected.end(); ++it)
	{
	    MapTile* tile = qgraphicsitem_cast<MapTile*>(*it);

    	    if(tile)
		tile->setTileSprite(EditorTheme::startFilledTile(ground), 0);
	}

	// fixed border
	const QSize & tileSize = EditorTheme::tileSize();
        QRectF rectArea = selectionArea().boundingRect();
	QPoint tile2(tileSize.width() / 2, tileSize.height() / 2);

	rectArea.setTopLeft(rectArea.topLeft() - tile2);
	rectArea.setBottomRight(rectArea.bottomRight() + tile2);

	QList<QGraphicsItem*> listItems = items(rectArea);

	for(QList<QGraphicsItem*>::iterator
	    it = listItems.begin(); it != listItems.end(); ++it)
	{
	    MapTile* tile = qgraphicsitem_cast<MapTile*>(*it);

    	    if(tile)
	    {
		QPair<int, int> indexGroundRotate = EditorTheme::groundBoundariesFix(*tile, mapTiles);

		if(0 <= indexGroundRotate.first)
            	    tile->setTileSprite(indexGroundRotate.first, indexGroundRotate.second);
	    }
	}

	emit dataModified();
    }
}

void MapData::removeObjectsAction(QAction* act)
{
    QList<QGraphicsItem*> selected = selectedItems();

    if(act)
    {
        //int type = act->data().toInt();
	QList<QGraphicsItem*> selected = selectedItems();

	for(QList<QGraphicsItem*>::iterator
	    it = selected.begin(); it != selected.end(); ++it)
	{
	    // code: remove objects
	}

	emit dataModified();
    }
}

void MapData::newMap(const QSize & msz, const QString &)
{
    const QSize & tileSize = EditorTheme::tileSize();

    mapTiles.newMap(msz);
    mapTiles.insertToScene(*this);

    setSceneRect(QRect(QPoint(0, 0),
	QSize(size().width() * tileSize.width(), size().height() * tileSize.height())));

    generateMiniMap();
}

bool MapData::loadMap(const QString & mapFile)
{
    qDebug() << "MapData::loadMap:" << mapFile;

    if(! loadMapMP2(mapFile) && ! loadMapXML(mapFile))
	return false;

    mapTiles.insertToScene(*this);

    const QSize & tileSize = EditorTheme::tileSize();
    setSceneRect(QRect(QPoint(0, 0),
	QSize(size().width() * tileSize.width(), size().height() * tileSize.height())));

    generateMiniMap();

    return true;
}

bool MapData::loadMapMP2(const QString & mapFile)
{
    MP2Format mp2;

    if(! mp2.loadMap(mapFile))
	return false;

    // import tiles
    if(! mapTiles.importMap(mp2.size, mp2.tiles, mp2.sprites))
	return false;

    mapName = mp2.name;
    mapDescription = mp2.description;
    mapStartWithHero = mp2.startWithHero;

    switch(mp2.difficulty)
    {
	case 0:		mapDifficulty = Difficulty::Easy; break;
	case 2:		mapDifficulty = Difficulty::Tough; break;
	case 3:		mapDifficulty = Difficulty::Expert; break;
	default:	mapDifficulty = Difficulty::Normal; break;
    }

    if(mp2.kingdomColor[0]) mapKingdomColors |= Color::Blue;
    if(mp2.kingdomColor[1]) mapKingdomColors |= Color::Green;
    if(mp2.kingdomColor[2]) mapKingdomColors |= Color::Red;
    if(mp2.kingdomColor[3]) mapKingdomColors |= Color::Yellow;
    if(mp2.kingdomColor[4]) mapKingdomColors |= Color::Orange;
    if(mp2.kingdomColor[5]) mapKingdomColors |= Color::Purple;

    if(mp2.humanAllow[0]) mapHumanColors |= Color::Blue;
    if(mp2.humanAllow[1]) mapHumanColors |= Color::Green;
    if(mp2.humanAllow[2]) mapHumanColors |= Color::Red;
    if(mp2.humanAllow[3]) mapHumanColors |= Color::Yellow;
    if(mp2.humanAllow[4]) mapHumanColors |= Color::Orange;
    if(mp2.humanAllow[5]) mapHumanColors |= Color::Purple;

    if(mp2.compAllow[0]) mapCompColors |= Color::Blue;
    if(mp2.compAllow[1]) mapCompColors |= Color::Green;
    if(mp2.compAllow[2]) mapCompColors |= Color::Red;
    if(mp2.compAllow[3]) mapCompColors |= Color::Yellow;
    if(mp2.compAllow[4]) mapCompColors |= Color::Orange;
    if(mp2.compAllow[5]) mapCompColors |= Color::Purple;

    switch(mp2.conditionWins)
    {
	case 1:		mapConditionWins.set(Conditions::CaptureTown, QPoint(mp2.conditionWinsData3, mp2.conditionWinsData4)); break;
	case 2:		mapConditionWins.set(Conditions::DefeatHero, QPoint(mp2.conditionWinsData3, mp2.conditionWinsData4)); break;
	case 3:		mapConditionWins.set(Conditions::FindArtifact, static_cast<int>(mp2.conditionWinsData3)); break;
	case 4:		mapConditionWins.set(Conditions::SideWins, static_cast<int>(mp2.conditionWinsData3)); break;
	case 5:		mapConditionWins.set(Conditions::AccumulateGold, 1000 * static_cast<int>(mp2.conditionWinsData3)); break;
	default:	mapConditionWins.set(Conditions::Wins); break;
    }

    if(mp2.conditionWinsData1)
	mapConditionWins.first |= Conditions::CompAlsoWins;

    if(mp2.conditionWinsData2)
	mapConditionWins.first |= Conditions::AllowNormalVictory;

    switch(mp2.conditionLoss)
    {
	case 1:		mapConditionLoss.set(Conditions::LoseTown, QPoint(mp2.conditionLossData1, mp2.conditionLossData2)); break;
	case 2:		mapConditionLoss.set(Conditions::LoseHero, QPoint(mp2.conditionLossData1, mp2.conditionLossData2)); break;
	case 3:		mapConditionLoss.set(Conditions::OutTime, static_cast<int>(mp2.conditionLossData1)); break;
	default:	mapConditionLoss.set(Conditions::Loss); break;
    }

    mapArea.uniq = mp2.uniq + 1;

    // import towns
    mapArea.importMP2Towns(mp2.castles);

    // import heroes
    mapArea.importMP2Heroes(mp2.heroes);

    // import signs
    mapArea.importMP2Signs(mp2.signs);

    // import map events
    mapArea.importMP2MapEvents(mp2.mapEvents);

    // import sphinx riddles
    mapArea.importMP2SphinxRiddles(mp2.sphinxes);

    // import day events
    for(QVector<mp2dayevent_t>::const_iterator
	    it = mp2.dayEvents.begin(); it != mp2.dayEvents.end(); ++it)
	    mapDayEvents.push_back(DayEvent(*it));

    // import rumors
    for(QVector<mp2rumor_t>::const_iterator
	    it = mp2.rumors.begin(); it != mp2.rumors.end(); ++it)
	    tavernRumors << Rumor(*it);

    return true;
}

bool MapData::loadMapXML(const QString & mapFile)
{
    QFile file(mapFile);
    QDomDocument dom;

    if(file.open(QIODevice::ReadOnly))
    {
	QString errorStr;
        int errorLine;
        int errorColumn;

        if(! dom.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
    	    qDebug() << errorStr << errorLine << errorColumn;
            file.close();
            return false;
        }
    }
    else
    { qDebug() << "error open " << mapFile;  return false; }
    file.close();

    QDomElement emap = dom.firstChildElement("map");

    if(emap.isNull())
    { qDebug() << "unknown format map";  return false; }

    int version = emap.hasAttribute("version") ? emap.attribute("version").toInt() : 0;
    if(version < FH2ENGINE_LAST_VERSION)
    {
	QApplication::restoreOverrideCursor();
	QMessageBox::warning(NULL, "Map Editor", "Unsupported map format.");
	return false;
    }
    mapVersion = version;

    emap >> *this;

    return true;
}

bool MapData::saveMapXML(const QString & mapFile) const
{
    QFile file(mapFile);

    if(! file.open(QIODevice::WriteOnly))
	return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QDomDocument doc;

    QDomElement emap = doc.createElement("map");
    emap.setAttribute("version", engineVersion);
    doc.appendChild(emap);

    emap << *this;

    doc.insertBefore(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), doc.firstChild());

    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    doc.save(out, 5, QDomNode::EncodingFromTextStream);

    QApplication::restoreOverrideCursor();

    return true;
}

QPoint MapData::mapToTile(const QPoint & pt) const
{
    const MapTile* tile = qgraphicsitem_cast<const MapTile*>(itemAt(pt));
    return tile ? tile->mapPos() : QPoint(-1, -1);
}

QRect MapData::mapToTile(const QRect & rt) const
{
    return QRect(mapToTile(rt.topLeft()), mapToTile(rt.bottomRight()));
}

bool MP2Format::loadMap(const QString & mapFile)
{
    H2::File map(mapFile);

    if(map.open(QIODevice::ReadOnly))
    {
	// 4 byte: orig ver
	if(map.readLE32() != 0x0000005C)
	{
	    qDebug() << "Incorrect map file: " << mapFile;
    	    map.close();
    	    return false;
	}

	// difficulty: 0: easy, 1: normal, 2: hard, 3: expert
	difficulty = map.readLE16();

	// width, height
	size.setWidth(map.readByte());
	size.setHeight(map.readByte());

	// kingdom color: blue, gree, red, yellow, orange, purple
	for(int ii = 0; ii < 6; ++ii)
	    kingdomColor[ii] = map.readByte();

	// allow human: blue, gree, red, yellow, orange, purple
	for(int ii = 0; ii < 6; ++ii)
	    humanAllow[ii] = map.readByte();

	// allow comp: blue, gree, red, yellow, orange, purple
	for(int ii = 0; ii < 6; ++ii)
	    compAllow[ii] = map.readByte();

	// wins
	map.seek(0x1D);
	conditionWins = map.readByte();

	// data wins
	conditionWinsData1 = map.readByte();
	conditionWinsData2 = map.readByte();
	conditionWinsData3 = map.readLE16();
	map.seek(0x2C);
	conditionWinsData4 = map.readLE16();

	// loss
	map.seek(0x22);
	conditionLoss = map.readByte();
	// data loss
	conditionLossData1 = map.readLE16();
	map.seek(0x2E);
	conditionLossData2 = map.readLE16();

	// start with hero
	map.seek(0x25);
	startWithHero = (0 == map.readByte());

	// race color
	for(int ii = 0; ii < 6; ++ii)
	    raceColor[ii] = map.readByte();

	// name
	map.seek(0x3A);
	name = map.readString(16);

	// description
	map.seek(0x76);
	description = map.readString(143);

	// data map: width, heigth
	map.seek(0x01A4);
	if(static_cast<int>(map.readLE32()) != size.width())
	{
	    qDebug() << "MP2Format::loadMap:" << "incorrect size";
    	    return false;
	}
	if(static_cast<int>(map.readLE32()) != size.height())
	{
	    qDebug() << "MP2Format::loadMap:" << "incorrect size";
    	    return false;
	}

	// data map: mp2tile, part1
	// count blocks: width * heigth
	tiles.resize(size.width() * size.height());

	for(QVector<mp2til_t>::iterator
	    it = tiles.begin(); it != tiles.end(); ++it)
	{
	    QDataStream ds(map.read(20));
	    ds >> (*it);
	}

	// data map: mp2ext, part2
	// count blocks: 4 byte
	sprites.resize(map.readLE32());

	for(QVector<mp2ext_t>::iterator
	    it = sprites.begin(); it != sprites.end(); ++it)
	{
	    QDataStream ds(map.read(15));
	    ds >> (*it);
	}

	// cood castles
	QVector<mp2pos_t> townPosBlocks;
	townPosBlocks.reserve(72);

	// 72 x 3 byte (px, py, id)
	for(int ii = 0; ii < 72; ++ii)
	{
	    mp2pos_t twn;

	    twn.posx = map.readByte();
	    twn.posy = map.readByte();
	    twn.type = map.readByte();

	    if(0xFF != twn.posx && 0xFF != twn.posy)
		townPosBlocks.push_back(twn);
	}

	// cood resource kingdoms
	QVector<mp2pos_t> resourcePosBlocks;
	resourcePosBlocks.reserve(144);

	// 144 x 3 byte (px, py, id)
	for(int ii = 0; ii < 144; ++ii)
	{
	    mp2pos_t res;

	    res.posx = map.readByte();
	    res.posy = map.readByte();
	    res.type = map.readByte();

	    if(0xFF != res.posx && 0xFF != res.posy)
		resourcePosBlocks.push_back(res);
	}

	// byte: numObelisks
	map.readByte();

	// find count latest blocks: unknown byte ?? ?? ?? LO HI 00 00
	int blocksCount = 0;

	while(1)
	{
    	    quint8 lo = map.readByte();
    	    quint8 hi = map.readByte();

    	    if(0 == hi && 0 == lo)
		break;
    	    else
    		blocksCount = 256 * hi + lo - 1;
	}

	// read latest blocks
	for(int ii = 0; ii < blocksCount; ++ii)
	{
	    // parse block
	    QByteArray block = map.readBlock(map.readLE16());
	    QDataStream data(block);
	    data.setByteOrder(QDataStream::LittleEndian);
	    const QPoint posBlock = positionExtBlockFromNumber(ii + 1);
	    const int posMapIndex = posBlock.x() < 0 ? -1 : size.width() * posBlock.y() + posBlock.x();

	    if(0 <= posMapIndex && posMapIndex < tiles.size())
	    {
		switch(tiles[posMapIndex].objectID)
		{
		    case 0x82: // sign,bottle block: 10 byte
		    case 0xDD:
			if(10 <= block.size() && 0x01 == block.at(0))
			{
			    mp2sign_t sign; data >> sign;
			    signs.push_back(H2::SignPos(sign, posBlock));
			}
			break;

		    case 0x93: // map event block: 50 byte
			if(50 <= block.size() && 0x01 == block.at(0))
			{
			    mp2mapevent_t event; data >> event;
			    mapEvents.push_back(H2::EventPos(event, posBlock));
			}
			break;

		    case 0xA3: // castle, rnd town, rnd castle block: 70 byte
		    case 0xB0:
		    case 0xB1:
			if(block.size() == 70)
			{
			    mp2town_t castle; data >> castle;
			    castles.push_back(H2::TownPos(castle, posBlock));
			}
			break;

		    case 0xB7: // hero, jail block: 76 byte
		    case 0xFB:
			if(block.size() == 76)
			{
			    mp2hero_t hero; data >> hero;
			    heroes.push_back(H2::HeroPos(hero, posBlock));
			}
			break;

		    case 0xCF: // sphinx block: 138 byte
			if(138 <= block.size() && 0 == block.at(0))
			{
			    mp2sphinx_t sphinx; data >> sphinx;
			    sphinxes.push_back(H2::SphinxPos(sphinx, posBlock));
			}
			break;

		    default: break;
		}
	    }
	    else
	    if(block.at(0) == 0)
	    {
		// day event block: 50 byte
		if(50 <= block.size() && 0x01 == block.at(42))
		{
		    mp2dayevent_t event; data >> event;
		    dayEvents.push_back(event);
		}
		else
		// rumor block: 9 byte
		if(9 <= block.size())
		{
		    mp2rumor_t rumor; data >> rumor;
		    if(rumor.text.isEmpty())
    			qDebug() << "MP2Format::loadMap:" <<"skip empty rumor, block: " << ii;
		    else
			rumors.push_back(rumor);
		}
    		else
		    qCritical() << "MP2Format::loadMap:" <<"unknown block: " << ii << ", size: " << block.size();
	    }
	    else
	     qCritical() << "MP2Format::loadMap:" <<"unknown block: " << ii << ", size: " << block.size() << ", byte: " << block[0];
	}

	uniq = map.readLE32();
	map.close();

	//
	return true;
    }

    return false;
}

QPoint MP2Format::positionExtBlockFromNumber(int num) const
{
    for(int yy = 0; yy < size.height(); ++yy)
    {
        for(int xx = 0; xx < size.width(); ++xx)
        {
            const mp2til_t & mp2 = tiles[xx + yy * size.width()];

	    switch(mp2.objectID)
	    {
		case 0x82: // sign, bottle block
		case 0xDD:
		case 0x93: // map event block
		case 0xA3: // castle, rnd town, rnd castle block
		case 0xB0:
		case 0xB1:
		case 0xB7: // hero, jail block
		case 0xFB:
		case 0xCF: // sphinx block
		    break;

		default: continue;
	    }

            quint16 orders = mp2.quantity2;
            orders <<= 8;
            orders |= mp2.quantity1;

            if(orders && !(orders % 0x08) && (num == orders / 0x08))
		return QPoint(xx, yy);
        }
    }

    return QPoint(-1, -1);
}

QDomElement & operator<< (QDomElement & emap, const MapData & data)
{
    DefaultValues defs;
    QDomDocument doc = emap.ownerDocument();
    QDomElement eheader = doc.createElement("header");
    emap.appendChild(eheader);

    eheader.setAttribute("localtime", QDateTime::currentDateTime().toTime_t());

    eheader.appendChild(doc.createElement("size")).appendChild(doc.createTextNode(data.mapTiles.sizeDescription()));
    eheader.appendChild(doc.createElement("name")).appendChild(doc.createTextNode(data.mapName));
    eheader.appendChild(doc.createElement("description")).appendChild(doc.createTextNode(data.mapDescription));
    eheader.appendChild(doc.createElement("authors")).appendChild(doc.createTextNode(data.mapAuthors));
    eheader.appendChild(doc.createElement("license")).appendChild(doc.createTextNode(data.mapLicense));
    eheader.appendChild(doc.createElement("difficulty")).appendChild(doc.createTextNode(QString::number(data.mapDifficulty)));

    QDomElement eplayers = doc.createElement("players");
    eplayers.setAttribute("kingdoms", data.mapKingdomColors);
    eplayers.setAttribute("humans", data.mapHumanColors);
    eplayers.setAttribute("computers", data.mapCompColors);
    eplayers.setAttribute("startWithHero", data.mapStartWithHero);
    eheader.appendChild(eplayers);

    QDomElement ewins = doc.createElement("conditionWins");
    eheader.appendChild(ewins);
    ewins << data.mapConditionWins;

    QDomElement eloss = doc.createElement("conditionLoss");
    eheader.appendChild(eloss);
    eloss << data.mapConditionLoss;

    QDomElement erumors = doc.createElement("rumors");
    emap.appendChild(erumors);
    erumors << data.tavernRumors;

    QDomElement events = doc.createElement("events");
    emap.appendChild(events);
    events << data.mapDayEvents;

    emap << data.mapArea;

    QDomElement edefaults = doc.createElement("defaults");
    emap.appendChild(edefaults);
    edefaults << defs;

    return emap;
}

QDomElement & operator>> (QDomElement & emap, MapData & data)
{
    DefaultValues defs;
    QDomElement eheader = emap.firstChildElement("header");

    //int loctime = eheader.hasAttribute("localtime") ? eheader.attribute("localtime").toInt() : 0;

    data.mapName = eheader.firstChildElement("name").text();
    data.mapDescription = eheader.firstChildElement("description").text();
    data.mapAuthors = eheader.firstChildElement("authors").text();
    data.mapLicense = eheader.firstChildElement("license").text();
    data.mapDifficulty = eheader.firstChildElement("difficulty").text().toInt();

    QDomElement eplayers = eheader.firstChildElement("players");

    data.mapKingdomColors = eplayers.attribute("kingdoms").toInt();
    data.mapHumanColors = eplayers.attribute("humans").toInt();
    data.mapCompColors = eplayers.attribute("computers").toInt();
    data.mapStartWithHero = eplayers.attribute("startWithHero").toInt();

    QDomElement ewins = eheader.firstChildElement("conditionWins");
    ewins >> data.mapConditionWins;

    QDomElement eloss = eheader.firstChildElement("conditionLoss");
    eloss >> data.mapConditionLoss;

    QDomElement erumors = emap.firstChildElement("rumors");
    erumors >> data.tavernRumors;

    QDomElement events = emap.firstChildElement("events");
    events >> data.mapDayEvents;

    emap >> data.mapArea;

    QDomElement edefaults = emap.firstChildElement("defaults");
    edefaults >> defs;

    return emap;
}


void MapData::selectObjectImage(void)
{
    Form::SelectImageObject form;

    if(QDialog::Accepted == form.exec())
    {
        currentObject = CompositeObjectCursor(form.result);
	update();
    }
}

void MapData::showMapOptions(void)
{
    Form::MapOptions form(*this);

    if(QDialog::Accepted == form.exec())
    {
	// tab1
	mapName = form.lineEditName->text();
	mapDescription = form.plainTextEditDescription->toPlainText();
	mapDifficulty = qvariant_cast<int>(comboBoxCurrentData(form.comboBoxDifficulty));

	// tab2
	mapConditionWins.set(qvariant_cast<int>(comboBoxCurrentData(form.comboBoxWinsCond)), comboBoxCurrentData(form.comboBoxWinsCondExt));
	mapConditionWins.setAllowNormalVictory(form.checkBoxAllowNormalVictory->isChecked());
	mapConditionWins.setCompAlsoWins(form.checkBoxCompAlsoWins->isChecked());
	mapConditionLoss.set(qvariant_cast<int>(comboBoxCurrentData(form.comboBoxLossCond)), comboBoxCurrentData(form.comboBoxLossCondExt));
	mapStartWithHero = form.checkBoxStartWithHero->isChecked();

	mapCompColors = 0;
	mapHumanColors = 0;

	for(QVector<Form::PlayerStatus*>::const_iterator
    	    it = form.labelPlayers.begin(); it != form.labelPlayers.end(); ++it)
	{
	    if(mapKingdomColors & (*it)->color())
	    {
		/* 0: n/a, 1: human only, 2: comp only, 3: comp or human */
		if(0x01 & (*it)->status())
		    mapHumanColors |= (*it)->color();
		if(0x02 & (*it)->status())
		    mapCompColors |= (*it)->color();
	    }
	}

	// tab3
	tavernRumors = form.listWidgetRumors->results();
        mapDayEvents = form.listWidgetEvents->results();

	// tab4
	mapAuthors = form.plainTextEditAuthors->toPlainText();
        mapLicense = form.plainTextEditLicense->toPlainText();

        emit dataModified();
    }
}

const DayEvents & MapData::dayEvents(void) const
{
    return mapDayEvents;
}

void MapData::generateMiniMap(void)
{
    MapWindow* mapWindow = qobject_cast<MapWindow*>(parent());

    if(mapWindow && mapWindow->miniMapWidget())
	mapWindow->miniMapWidget()->generateFromTiles(mapTiles);
}

void MapData::editObjectAttributes(void)
{
    if(tileOverMouse)
	switch(tileOverMouse->object())
    {
    	case MapObj::Resource:	editResourceDialog(*tileOverMouse); break;
    	case MapObj::Event:	editMapEventDialog(*tileOverMouse); break;
    	case MapObj::RndCastle:
    	case MapObj::RndTown:
    	case MapObj::Castle:	editTownDialog(*tileOverMouse); break;
    	case MapObj::Bottle:
    	case MapObj::Sign:	editSignDialog(*tileOverMouse); break;
    	case MapObj::Heroes:	editHeroDialog(*tileOverMouse); break;
	case MapObj::Sphinx:	editSphinxDialog(*tileOverMouse); break;

    	default:
	{
	    Form::ObjectEventsDialog form;

	    if(QDialog::Accepted == form.exec())
	    {
	    }
	
	    //QMessageBox::information(qobject_cast<MapWindow*>(parent()), "Object Attributes",
	    //	    "Sorry!\nChange attributes of the object is not yet available."); break;
	}
    }
}

void MapData::removeCurrentObject(void)
{
    if(tileOverMouse)
    {
	// remove object info
	MapObject* obj = mapObjects.find(tileOverMouse->mapPos()).data();
	if(obj) mapObjects.remove(obj->uid());

	// remove sprites
	if(! tileOverMouse->levels1().empty())
	{
    	    // get uid from level 1 (last sprite)
	    mapTiles.removeSprites(tileOverMouse->levels1().back().uid());
	    update();
	}

	emit dataModified();
    }
}

void MapData::editMapEventDialog(const MapTile & tile)
{
    MapEvent* event = dynamic_cast<MapEvent*>(mapObjects.find(tile.mapPos()).data());

    if(event)
    {
	Form::MapEventDialog form(*event, mapKingdomColors);

	if(QDialog::Accepted == form.exec())
	{
	    *event = form.result(event->pos(), event->uid());

	    emit dataModified();
	}
    }
}

void MapData::editResourceDialog(const MapTile & tile)
{
    const MapTileExt* ext = tile.levels1().find(MapTileExt::isResource);

    if(ext)
    {
	MapResource* res = dynamic_cast<MapResource*>(mapObjects.find(tile.mapPos()).data());
	Form::EditResourceDialog form(res ? res->type : MapTileExt::resource(*ext), res ? res->count : 0);

	if(QDialog::Accepted == form.exec())
	{
	    if(form.checkBoxDefault->isChecked())
		mapObjects.remove(tile.mapPos());
	    else
		mapObjects.push_back(new MapResource(tile.mapPos(), ext->uid(), MapTileExt::resource(*ext), form.spinBoxCount->value()));

	    emit dataModified();
	}
    }
}

void MapData::editTownDialog(const MapTile & tile)
{
    MapTown* town = dynamic_cast<MapTown*>(mapObjects.find(tile.mapPos()).data());

    if(town)
    {
	Form::TownDialog form(*town);

	if(QDialog::Accepted == form.exec())
	{
	    town->nameTown = form.lineEditName->text();
	    town->buildings = form.buildings() | form.dwellings();
	    town->troops = form.troops();
	    town->forceTown = form.checkBoxAllowCastle->isChecked();
	    town->customBuilding = ! form.checkBoxBuildingsDefault->isChecked();
	    //town->color = ;
	    //town->race = ;

	    emit dataModified();
	}
    }
}

void MapData::editHeroDialog(const MapTile & tile)
{
    MapHero* hero = dynamic_cast<MapHero*>(mapObjects.find(tile.mapPos()).data());

    if(hero)
    {
	Form::HeroDialog form(*hero);

	if(QDialog::Accepted == form.exec())
	{
	    hero->nameHero = form.lineEditName->text();
	    hero->troops = form.troops();
	    hero->portrait = form.verticalScrollBarPort->value();
	    hero->artifacts = form.artifacts();
	    hero->experience = form.lineEditExperience->text().toInt();
	    hero->patrolMode = form.checkBoxEnablePatrol->isChecked();
	    hero->patrolSquare = form.comboBoxPatrol->itemData(form.comboBoxPatrol->currentIndex()).toInt();
	    hero->skills = form.skills();
	    //hero->color = ;
	    //hero->race = ;

	    emit dataModified();
	}
    }
}

void MapData::editSphinxDialog(const MapTile & tile)
{
    MapSphinx* sphinx = dynamic_cast<MapSphinx*>(mapObjects.find(tile.mapPos()).data());

    if(sphinx)
    {
	Form::MapSphinxDialog form(*sphinx);

	if(QDialog::Accepted == form.exec())
	{
	    *sphinx = form.result(sphinx->pos(), sphinx->uid());

	    emit dataModified();
	}
    }
}

void MapData::editSignDialog(const MapTile & tile)
{
    MapSign* sign = dynamic_cast<MapSign*>(mapObjects.find(tile.mapPos()).data());

    if(sign)
    {
	Form::SignDialog form(sign->message);

	if(QDialog::Accepted == form.exec())
	{
	    sign->message = form.plainTextEdit->toPlainText();

	    emit dataModified();
	}
    }
}


void MapData::addMapObject(const QPoint & pos, const CompositeObject & obj, quint32 uid)
{
    const QSize & tileSize = EditorTheme::tileSize();

    // add sprites section
    for(CompositeObject::const_iterator
	it = obj.begin(); it != obj.end(); ++it)
    {
	QPoint offset((*it).spritePos.x() * tileSize.width() + 1, (*it).spritePos.y() * tileSize.height() + 1);
	MapTile* tile = mapTiles.mapToTile(pos + offset);
	tile->addSpriteSection(obj, *it, uid);

	// add object info
	if((*it).spriteLevel == SpriteLevel::Action)
	{
	    MapObject* objPtr = NULL;

	    switch(obj.classId)
	    {
		case MapObj::Bottle:
		case MapObj::Sign:	objPtr = new MapSign(tile->mapPos(), uid); break;
    		case MapObj::Resource:	objPtr = new MapResource(tile->mapPos(), uid); break;
		case MapObj::Event:	objPtr = new MapEvent(tile->mapPos(), uid); break;
    		case MapObj::RndCastle:
    		case MapObj::RndTown:
    		case MapObj::Castle:	objPtr = new MapTown(tile->mapPos(), uid); break;
    		case MapObj::Heroes:	objPtr = new MapHero(tile->mapPos(), uid); break;
    		case MapObj::Sphinx:	objPtr = new MapSphinx(tile->mapPos(), uid); break;
		default: break;
	    }

	    if(objPtr)
		mapObjects.push_back(objPtr);
	}
    }

    emit dataModified();
}
