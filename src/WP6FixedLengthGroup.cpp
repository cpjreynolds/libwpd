/* libwpd2
 * Copyright (C) 2002 William Lachance (wlach@interlog.com)
 * Copyright (C) 2002 Marc Maurer (j.m.maurer@student.utwente.nl)
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
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

#include "WP6FixedLengthGroup.h"
#include "WP6FileStructure.h"
#include "WP6AttributeOnGroup.h"
#include "WP6AttributeOffGroup.h"

WP6FixedLengthGroup::WP6FixedLengthGroup(FILE * stream)
	: WP6Part(stream)
{
}

WP6FixedLengthGroup * WP6FixedLengthGroup::constructFixedLengthGroup(FILE * stream, guint8 groupID)
{
#define WP6_TOP_EXTENDED_CHARACTER 0xF0
#define WP6_TOP_UNDO_GROUP 0xF1
#define WP6_TOP_ATTRIBUTE_ON 0xF2
#define WP6_TOP_ATTRIBUTE_OFF 0xF3	
	
	switch (groupID)
	{
		case WP6_TOP_EXTENDED_CHARACTER: 
			return new WP6FixedLengthGroup(stream);
		case WP6_TOP_UNDO_GROUP:
			return new WP6FixedLengthGroup(stream);
		case WP6_TOP_ATTRIBUTE_ON:
			return new WP6AttributeOnGroup(stream);
		case WP6_TOP_ATTRIBUTE_OFF:
			return new WP6AttributeOffGroup(stream);
		default:
			// should not happen
			return new WP6FixedLengthGroup(stream);
	}
}

gboolean WP6FixedLengthGroup::parse()
{
	guint32 startPosition = ftell(m_pStream);
	
	WPD_CHECK_INTERNAL_ERROR( _parseContents() );
	
	WPD_CHECK_FILE_SEEK_ERROR(fseek(m_pStream, (startPosition + m_iSize - 1 - ftell(m_pStream)), SEEK_CUR));
}