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

#include <map>
#include <list>
#include <string>

#include "engine.h"

u32 crc32b(const char* msg)
{
    u32 crc = 0xFFFFFFFF;
    u32 index = 0;

    while(msg[index])
    {
	crc ^= static_cast<u32>(msg[index]);

	for(int bit = 0; bit < 8; ++bit)
	{
	    size_t mask = -(crc & 1);
	    crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }

	++index;
    }

   return ~crc;
}

struct chunk
{
    u32		offset;
    u32		length;

    chunk() : offset(0), length(0) {}
    chunk(u32 off, u32 len) : offset(off), length(len) {}
};

struct mofile
{
    u32 count, offset_strings1, offset_strings2, hash_size, hash_offset;
    StreamBuf buf;
    std::map<u32, chunk> hash_offsets;
    std::string encoding;
    std::string plural_forms;
    u32 nplurals;

    mofile() : count(0), offset_strings1(0), offset_strings2(0), hash_size(0), hash_offset(0), nplurals(0) {}

    const char* ngettext(const char* str, size_t plural)
    {
	std::map<u32, chunk>::const_iterator it = hash_offsets.find(crc32b(str));
	if(it == hash_offsets.end())
	    return str;

	const u8* ptr = buf.data() + (*it).second.offset;

	while(plural > 0)
	{
	    while(*ptr) ptr++;
	    plural--;
	    ptr++;
	}
	return reinterpret_cast<const char*>(ptr);
    }

    std::string get_tag(const std::string & str, const std::string & tag, const std::string & sep)
    {
	std::string res;
	if(str.size() > tag.size() &&
	    tag == str.substr(0, tag.size()))
	{
	    size_t pos = str.find(sep);
	    if(pos != std::string::npos)
	    res = str.substr(pos + sep.size());
	}
	return res;
    }

    bool open(const std::string & file)
    {
	StreamFile sf;

	if(! sf.open(file, "rb"))
	    return false;
	else
	{
	    u32 size = sf.size();
	    u32 id = 0;
	    sf >> id;

	    if(0x950412de != id)
	    {
		ERROR("incorrect mo id: " << GetHexString(id));
		return false;
	    }
	    else
	    {
		u16 major, minor;
		sf >> major >> minor;

		if(0 != major)
		{
		    ERROR("incorrect major version: " << GetHexString(major, 4));
		    return false;
		}
		else
		{
		    sf >> count >> offset_strings1 >> offset_strings2 >> hash_size >> hash_offset;

		    buf = StreamBuf(size);

		    sf.seek(0);
		    sf.read(buf.data(), buf.size());
		    sf.close();
		}
	    }
	}

	// parse encoding and plural forms
	if(count)
	{
	    buf.seek(offset_strings2);
	    u32 length2 = buf.get32();
	    u32 offset2 = buf.get32();

	    const std::string tag1("Content-Type");
	    const std::string sep1("charset=");
	    const std::string tag2("Plural-Forms");
	    const std::string sep2(": ");

	    std::list<std::string> tags = StringSplit(std::string(reinterpret_cast<const char*>(buf.data() + offset2),
								reinterpret_cast<const char*>(buf.data() + offset2 + length2)), "\n");
	    for(std::list<std::string>::const_iterator
		it = tags.begin(); it != tags.end(); ++it)
	    {
		if(encoding.empty())
		    encoding = get_tag(*it, tag1, sep1);

		if(plural_forms.empty())
		    plural_forms = get_tag(*it, tag2, sep2);
	    }
	}

	// generate hash table
	for(u32 index = 0; index < count; ++index)
	{
	    buf.seek(offset_strings1 + index * 8 /* length, offset */ + 4 /* length unused */);
	    u32 offset1 = buf.get32();
	    u32 crc = crc32b(reinterpret_cast<const char*>(buf.data() + offset1));
	    buf.seek(offset_strings2 + index * 8 /* length, offset */);
	    u32 length2 = buf.get32();
	    u32 offset2 = buf.get32();
	    hash_offsets[crc] = chunk(offset2, length2);
	}

        return true;
    }

};

namespace translation
{
    enum { LOCALE_EN, LOCALE_CS, LOCALE_ES, LOCALE_FR, LOCALE_HU, LOCALE_NL, LOCALE_PL, LOCALE_PT, LOCALE_RU, LOCALE_SV, LOCALE_TR };

    mofile*				current = NULL;
    std::map<std::string, mofile>	domains;
    int					locale = LOCALE_EN;
    char				context = 0;

    void set_strip_context(char strip)
    {
	context = strip;
    }

    const char* strip_context(const char* str)
    {
	if(! context) return str;
	const char* pos = str;
	while(*pos && *pos++ != context);
	return *pos ? pos : str;
    }

    bool bind_domain(const char* domain, const char* file)
    {
	std::map<std::string, mofile>::const_iterator it = domains.find(domain);
	if(it != domains.end())
	    return true;

	System::SetMessageLocale("");
	std::string str = System::GetMessageLocale(1);

	if(str == "cs")	locale = LOCALE_CS;
	else
	if(str == "es")	locale = LOCALE_ES;
	else
	if(str == "fr")	locale = LOCALE_FR;
	else
	if(str == "hu")	locale = LOCALE_HU;
	else
	if(str == "nl")	locale = LOCALE_NL;
	else
	if(str == "pl")	locale = LOCALE_PL;
	else
	if(str == "pt")	locale = LOCALE_PT;
	else
	if(str == "ru")	locale = LOCALE_RU;
	else
	if(str == "sv")	locale = LOCALE_SV;
	else
	if(str == "tr")	locale = LOCALE_TR;

	return domains[domain].open(file);
    }

    bool set_domain(const char* domain)
    {
	std::map<std::string, mofile>::iterator it = domains.find(domain);
	if(it == domains.end())
	    return false;

	current = & (*it).second;
	return true;
    }

    const char* gettext(const char* str)
    {
	return strip_context(current ? current->ngettext(str, 0) : str);
    }

    const char* dgettext(const char* domain, const char* str)
    {
	set_domain(domain);
	return gettext(str);
    }

    const char* ngettext(const char* str, const char* plural, size_t n)
    {
	if(current)
	    switch(locale)
	{
	    case LOCALE_CS:
		return strip_context(current->ngettext(str, ((n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2)));
	    case LOCALE_ES:
		return strip_context(current->ngettext(str, (n != 1)));
	    case LOCALE_FR:
		return strip_context(current->ngettext(str, (n > 1)));
	    case LOCALE_HU:
		return strip_context(current->ngettext(str, (n != 1)));
	    case LOCALE_NL:
		return strip_context(current->ngettext(str, (n != 1)));
	    case LOCALE_PL:
		return strip_context(current->ngettext(str, (n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)));
	    case LOCALE_PT:
		return strip_context(current->ngettext(str, (n > 1)));
	    case LOCALE_RU:
		return strip_context(current->ngettext(str, (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)));
		break;
	    case LOCALE_SV:
		return strip_context(current->ngettext(str, (n != 1)));
	    case LOCALE_TR:
		return strip_context(current->ngettext(str, 0));
	    default: break;
	}

	return strip_context(n == 1 ? str : plural);
    }

    const char* dngettext(const char* domain, const char* str, const char* plural, size_t num)
    {
	set_domain(domain);
	return ngettext(str, plural, num);
    }
}
