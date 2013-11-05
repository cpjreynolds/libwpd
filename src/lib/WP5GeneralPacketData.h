/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#ifndef WP5GENERALPACKETDATA_H
#define WP5GENERALPACKETDATA_H
#include "WPXParser.h"
#include "libwpd_types.h"

class WP5GeneralPacketIndex;

class WP5GeneralPacketData
{
public:
	WP5GeneralPacketData();
	virtual ~WP5GeneralPacketData() {}

	static WP5GeneralPacketData *constructGeneralPacketData(librevenge::RVNGInputStream *input, WPXEncryption *encryption, WP5GeneralPacketIndex *packetIndex);

protected:
	virtual void _readContents(librevenge::RVNGInputStream *input, WPXEncryption *encryption, uint32_t dataSize) = 0;
	virtual void _read(librevenge::RVNGInputStream *input, WPXEncryption *encryption, uint32_t dataOffset, uint32_t dataSize);
};

#endif /* WP5GENERALPACKETDATA_H */
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
