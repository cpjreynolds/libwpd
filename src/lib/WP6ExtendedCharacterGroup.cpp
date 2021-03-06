/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2003 William Lachance (wrlach@gmail.com)
 * Copyright (C) 2003 Marc Maurer (uwog@uwog.net)
 * Copyright (C) 2002 Ariya Hidayat <ariyahidayat@yahoo.de>
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "WP6ExtendedCharacterGroup.h"
#include "WP6FileStructure.h"
#include "WP6Listener.h"
#include "libwpd_internal.h"

WP6ExtendedCharacterGroup::WP6ExtendedCharacterGroup(librevenge::RVNGInputStream *input, WPXEncryption *encryption, unsigned char groupID) :
	WP6FixedLengthGroup(groupID),
	m_character(0),
	m_characterSet(0)
{
	_read(input, encryption);
}

void WP6ExtendedCharacterGroup::_readContents(librevenge::RVNGInputStream *input, WPXEncryption *encryption)
{
	m_character = readU8(input, encryption);
	m_characterSet = readU8(input, encryption);
}

void WP6ExtendedCharacterGroup::parse(WP6Listener *listener)
{
	const unsigned *chars;
	int len = extendedCharacterWP6ToUCS4(m_character,
	                                     m_characterSet, &chars);
	int i;

	for (i = 0; i < len; i++)
		listener->insertCharacter(chars[i]);

}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
