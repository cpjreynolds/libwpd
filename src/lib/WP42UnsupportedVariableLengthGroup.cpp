/* libwpd
 * Copyright (C) 2003 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2003 Marc Maurer (j.m.maurer@student.utwente.nl)
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

#include "WP42UnsupportedVariableLengthGroup.h"
#include "libwpd_internal.h"

WP42UnsupportedVariableLengthGroup::WP42UnsupportedVariableLengthGroup(WPXInputStream *input, uint8_t group) :
	WP42VariableLengthGroup(group)
{
}

void WP42UnsupportedVariableLengthGroup::_readContents(WPXInputStream *input)
{
	WPD_DEBUG_MSG(("WordPerfect: Handling an unsupported variable length group\n"));
	_read(input);
};
