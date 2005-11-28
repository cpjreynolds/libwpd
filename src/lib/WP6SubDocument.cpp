/* libwpd
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#include "WP6SubDocument.h"
#include "WP6Parser.h"
#include "libwpd_internal.h"
#include "WP6Listener.h"

WP6SubDocument::WP6SubDocument(uint8_t * streamData, const int dataSize) :
	WPXSubDocument()
{
	m_stream = new WPXMemoryInputStream(streamData, dataSize);
}

void WP6SubDocument::parse(WPXListener *listener) const
{
	m_stream->seek(0, WPX_SEEK_SET);
	WP6Parser::parseDocument(m_stream, static_cast<WP6Listener *>(listener));
}
