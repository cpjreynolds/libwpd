/* libwpd2
 * Copyright (C) 2002 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2002 Marc Maurer (j.m.maurer@student.utwente.nl)
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

#ifndef WP6LLLISTENER_H
#define WP6LLLISTENER_H
#include "WPXLLListener.h"
#include "WP6PrefixDataPacket.h"

class WP6DefaultInitialFontPacket;
class WP6PrefixData;

enum WP6OutlineLocation { paragraphGroup, indexHeader };

class WP6LLListener : public WPXLLListener
{
 public:
	WP6LLListener() : WPXLLListener(), m_prefixData(NULL) {}
	virtual ~WP6LLListener() {}
	virtual void fontChange(const guint16 matchedFontPointSize, const guint16 fontPID) = 0;
	virtual void undoChange(const guint8 undoType, const guint16 undoLevel) = 0;
	virtual void updateOutlineDefinition(const WP6OutlineLocation outlineLocation, const guint16 outlineHash, 
					     const guint8 *numberingMethods, const guint8 tabBehaviourFlag) = 0;
	virtual void paragraphNumberOn(const guint16 outlineHash, const guint8 level, const guint8 flag) = 0;
	virtual void paragraphNumberOff() = 0;
	virtual void displayNumberReferenceGroupOn(const guint8 subGroup, const guint8 level) = 0;
	virtual void displayNumberReferenceGroupOff(const guint8 subGroup) = 0;
	virtual void styleGroupOn(const guint8 subGroup) = 0;
	virtual void styleGroupOff(const guint8 subGroup) = 0;
	void setPrefixData(WP6PrefixData *prefixData) { m_prefixData = prefixData; }
 protected:
	const WP6PrefixDataPacket * _getPrefixDataPacket(const int prefixID) const;
	const WP6DefaultInitialFontPacket * _getDefaultInitialFontPacket() const; 

 private:
	WP6PrefixData *m_prefixData;
};

#endif /* WP6LLLISTENER_H */
