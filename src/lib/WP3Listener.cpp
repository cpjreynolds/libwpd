/* libwpd
 * Copyright (C) 2004 Marc Maurer (j.m.maurer@student.utwente.nl)
 * Copyright (C) 2004-2005 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#include "WP3Listener.h"
#include "WP3FileStructure.h"
#include "WPXFileStructure.h"
#include "libwpd_internal.h"

_WP3ParsingState::_WP3ParsingState():
	m_colSpan(1),
	m_rowSpan(1),
	m_isTableCellToBeStarted(false)
{
}

_WP3ParsingState::~_WP3ParsingState()
{
}

WP3Listener::WP3Listener(std::vector<WPXPageSpan *> *pageList, WPXHLListenerImpl *listenerImpl) :
	WPXListener(pageList, listenerImpl),
	m_parseState(new WP3ParsingState)
{
	m_textBuffer.clear();
}

WP3Listener::~WP3Listener() 
{
	delete m_parseState;
}


/****************************************
 public 'HLListenerImpl' functions
*****************************************/

void WP3Listener::insertCharacter(const uint16_t character)
{
        if (!isUndoOn())
	{
		if (!m_ps->m_isSpanOpened)
			_openSpan();
		appendUCS4(m_textBuffer, (uint32_t)character);
	}
}

void WP3Listener::insertTab(const uint8_t tabType, const float tabPosition)
{
        if (!isUndoOn())
	{
		if (!m_ps->m_isSpanOpened)
			_openSpan();
		else
			_flushText();
		m_listenerImpl->insertTab();
	}
}

void WP3Listener::insertEOL()
{
	if (!isUndoOn())
	{
		if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
			_openSpan();
		if (m_ps->m_isParagraphOpened)
			_closeParagraph();
		if (m_ps->m_isListElementOpened)
			_closeListElement();
	}

}

void WP3Listener::endDocument()
{
	_closeSpan();
	_closeParagraph();
	_closeSection();
	_closePageSpan();
	m_listenerImpl->endDocument();
}

void WP3Listener::defineTable(uint8_t position, uint16_t leftOffset)
{
	if (!isUndoOn())
	{
		switch (position & 0x07)
		{
		case 0:
			m_ps->m_tableDefinition.m_positionBits = WPX_TABLE_POSITION_ALIGN_WITH_LEFT_MARGIN;
			break;
		case 1:
			m_ps->m_tableDefinition.m_positionBits = WPX_TABLE_POSITION_ALIGN_WITH_RIGHT_MARGIN;
			break;
		case 2:
			m_ps->m_tableDefinition.m_positionBits = WPX_TABLE_POSITION_CENTER_BETWEEN_MARGINS;
			break;
		case 3:
			m_ps->m_tableDefinition.m_positionBits = WPX_TABLE_POSITION_FULL;
			break;
		case 4:
			m_ps->m_tableDefinition.m_positionBits = WPX_TABLE_POSITION_ABSOLUTE_FROM_LEFT_MARGIN;
			break;
		default:
			// should not happen
			break;
		}
		// Note: WordPerfect has an offset from the left edge of the page. We translate it to the offset from the left margin
		m_ps->m_tableDefinition.m_leftOffset = (float)((double)leftOffset / (double)WPX_NUM_WPUS_PER_INCH) - m_ps->m_paragraphMarginLeft;

		// remove all the old column information
		m_ps->m_tableDefinition.columns.clear();
		m_ps->m_tableDefinition.columnsProperties.clear();
	}
}

void WP3Listener::addTableColumnDefinition(uint32_t width, uint32_t leftGutter, uint32_t rightGutter, uint32_t attributes, uint8_t alignment)
{
	if (!isUndoOn())
	{
		// define the new column
		WPXColumnDefinition colDef;
		colDef.m_width = (float)((double)width / (double)WPX_NUM_WPUS_PER_INCH);
		colDef.m_leftGutter = (float)((double)width / (double)WPX_NUM_WPUS_PER_INCH);
		colDef.m_rightGutter = (float)((double)width / (double)WPX_NUM_WPUS_PER_INCH);

		// add the new column definition to our table definition
		m_ps->m_tableDefinition.columns.push_back(colDef);
		
		WPXColumnProperties colProp;
		colProp.m_attributes = attributes;
		colProp.m_alignment = alignment;
		
		m_ps->m_tableDefinition.columnsProperties.push_back(colProp);
	}
}

void WP3Listener::startTable()
{
	if (!isUndoOn())
	{
		// save the justification information. We will need it after the table ends.
		m_ps->m_paragraphJustificationBeforeTable = m_ps->m_paragraphJustification;
		if (m_ps->m_sectionAttributesChanged && !m_ps->m_isTableOpened)
		{
			_closeSection();
			_openSection();
			m_ps->m_sectionAttributesChanged = false;
		}
		_openTable();
	}
}

void WP3Listener::insertRow(const uint16_t rowHeight, const bool isMinimumHeight, const bool isHeaderRow)
{
	if (!isUndoOn())
	{
		_flushText();
		float rowHeightInch = (float)((double) rowHeight / (double)WPX_NUM_WPUS_PER_INCH);
		_openTableRow(rowHeightInch, isMinimumHeight, isHeaderRow);
	}
}

void WP3Listener::insertCell(const uint8_t colSpan, const uint8_t rowSpan, const bool boundFromLeft, const bool boundFromAbove,
			const uint8_t borderBits, const RGBSColor * cellFgColor, const RGBSColor * cellBgColor, 
			const RGBSColor * cellBorderColor, const WPXVerticalAlignment cellVerticalAlignment, 
			const bool useCellAttributes, const uint32_t cellAttributes)
{
	if (!isUndoOn())
	{
		m_parseState->m_isTableCellToBeStarted = true;
	}
}

void WP3Listener::setTableCellSpan(const uint16_t colSpan, const uint16_t rowSpan)
{
	if (!isUndoOn())
	{
		m_parseState->m_colSpan=colSpan;
		m_parseState->m_rowSpan=rowSpan;
	}
}

void WP3Listener::endTable()
{
	if (!isUndoOn())
	{
		_flushText();
		_closeTable();
		// restore the justification that was there before the table.
		m_ps->m_paragraphJustification = m_ps->m_paragraphJustificationBeforeTable;
		m_parseState->m_isTableCellToBeStarted = false;
	}
}


/****************************************
 public 'parser' functions
*****************************************/

void WP3Listener::attributeChange(const bool isOn, const uint8_t attribute)
{
        if (!isUndoOn())
	{
		_closeSpan();

		uint32_t textAttributeBit = 0;

		// FIXME: handle all the possible attribute bits
		switch (attribute)
		{
			case WP3_ATTRIBUTE_BOLD:
				textAttributeBit = WPX_BOLD_BIT;
				break;
			case WP3_ATTRIBUTE_ITALICS:
				textAttributeBit = WPX_ITALICS_BIT;
				break;
			case WP3_ATTRIBUTE_UNDERLINE:
				textAttributeBit = WPX_UNDERLINE_BIT;
				break;
			case WP3_ATTRIBUTE_OUTLINE:
				textAttributeBit = WPX_OUTLINE_BIT;
				break;
			case WP3_ATTRIBUTE_SHADOW:
				textAttributeBit = WPX_SHADOW_BIT;
				break;
			case WP3_ATTRIBUTE_REDLINE:
				textAttributeBit = WPX_REDLINE_BIT;
				break;
			case WP3_ATTRIBUTE_STRIKE_OUT:
				textAttributeBit = WPX_STRIKEOUT_BIT;
				break;
			case WP3_ATTRIBUTE_SUBSCRIPT:
				textAttributeBit = WPX_SUBSCRIPT_BIT;
				break;
			case WP3_ATTRIBUTE_SUPERSCRIPT:
				textAttributeBit = WPX_SUPERSCRIPT_BIT;
				break;
			case WP3_ATTRIBUTE_DOUBLE_UNDERLINE:
				textAttributeBit = WPX_DOUBLE_UNDERLINE_BIT;
				break;
			case WP3_ATTRIBUTE_EXTRA_LARGE:
				textAttributeBit = WPX_EXTRA_LARGE_BIT;
				break;
			case WP3_ATTRIBUTE_VERY_LARGE:
				textAttributeBit = WPX_VERY_LARGE_BIT;
				break;
			case WP3_ATTRIBUTE_LARGE:
				textAttributeBit = WPX_LARGE_BIT;
				break;
			case WP3_ATTRIBUTE_SMALL_PRINT:
				textAttributeBit = WPX_SMALL_PRINT_BIT;
				break;
			case WP3_ATTRIBUTE_FINE_PRINT:
				textAttributeBit = WPX_FINE_PRINT_BIT;
				break;		
			case WP3_ATTRIBUTE_SMALL_CAPS:
				textAttributeBit = WPX_SMALL_CAPS_BIT;
				break;
		}

		if (isOn)
			m_ps->m_textAttributeBits |= textAttributeBit;
		else
			m_ps->m_textAttributeBits ^= textAttributeBit;
	}
}

void WP3Listener::undoChange(const uint8_t undoType, const uint16_t undoLevel)
{
        if (undoType == 0x00) // begin invalid text
                m_isUndoOn = true;
        else if (undoType == 0x01) // end invalid text
                m_isUndoOn = false;
}

void WP3Listener::marginChange(uint8_t side, uint16_t margin)
{
	if (!isUndoOn())
	{
		float marginInch = (float)((double)margin/ (double)WPX_NUM_WPUS_PER_INCH);
		bool marginChanged = false;

		switch(side)
		{
		case WPX_LEFT:
			m_ps->m_leftMarginByPageMarginChange = marginInch - m_ps->m_pageMarginLeft;
			m_ps->m_paragraphMarginLeft = m_ps->m_leftMarginByPageMarginChange
						+ m_ps->m_leftMarginByParagraphMarginChange
						+ m_ps->m_leftMarginByTabs;
			break;
		case WPX_RIGHT:
			m_ps->m_rightMarginByPageMarginChange = marginInch - m_ps->m_pageMarginRight;
			m_ps->m_paragraphMarginRight = m_ps->m_rightMarginByPageMarginChange
						+ m_ps->m_rightMarginByParagraphMarginChange
						+ m_ps->m_rightMarginByTabs;
			break;
		}
	}
}

void WP3Listener::indentFirstLineChange(int16_t offset)
{
	if (!isUndoOn())
	{
		float offsetInch = (float)((double)offset / (double)WPX_NUM_WPUS_PER_INCH);
		m_ps->m_textIndentByParagraphIndentChange = offsetInch;
		// This is necessary in case we have Indent First Line and Hard Back Tab
		// in the same time. The Hard Back Tab applies to the current paragraph
		// only. Indent First Line applies untill an new Indent First Line code.
		m_ps->m_paragraphTextIndent = m_ps->m_textIndentByParagraphIndentChange
					+ m_ps->m_textIndentByTabs;
	}
}

void WP3Listener::beginningOfParagraphOff()
{
	if (!isUndoOn())
	{
		if (m_parseState->m_isTableCellToBeStarted)
		{
			if (m_ps->m_currentTableRow < 0) // cell without a row, invalid
				throw ParseException();
			_flushText();
			RGBSColor tmpCellBorderColor(0x00, 0x00, 0x00, 0x64);
			_openTableCell((uint8_t)m_parseState->m_colSpan, (uint8_t)m_parseState->m_rowSpan, false, false, 0x00000000,       
				       NULL, NULL, &tmpCellBorderColor, TOP);
			m_parseState->m_colSpan=1;
			m_parseState->m_rowSpan=1;
			m_parseState->m_isTableCellToBeStarted = false;
			m_ps->m_isCellWithoutParagraph = true;
			m_ps->m_cellAttributeBits = 0x00000000;
		}
	}
}

/****************************************
 private functions
*****************************************/

void WP3Listener::_flushText()
{
	if (m_textBuffer.len())
		m_listenerImpl->insertText(m_textBuffer);
	m_textBuffer.clear();
}
