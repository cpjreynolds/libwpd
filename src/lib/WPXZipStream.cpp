/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Copyright (C) 2012 Fridrich Strba (fridrich.strba@bluewin.ch)
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include <string>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include "WPXZipStream.h"
#include "WPXStreamImplementation.h"

namespace
{
class StreamException
{
};

struct LocalFileHeader
{
	unsigned short min_version;
	unsigned short general_flag;
	unsigned short compression;
	unsigned short lastmod_time;
	unsigned short lastmod_date;
	unsigned crc32;
	unsigned compressed_size;
	unsigned uncompressed_size;
	unsigned short filename_size;
	unsigned short extra_field_size;
	std::string filename;
	std::string extra_field;
	LocalFileHeader()
		: min_version(0), general_flag(0), compression(0), lastmod_time(0), lastmod_date(0),
		  crc32(0), compressed_size(0), uncompressed_size(0), filename_size(0), extra_field_size(0),
		  filename(), extra_field() {}
	~LocalFileHeader() {}
};

struct CentralDirectoryEntry
{
	unsigned short creator_version;
	unsigned short min_version;
	unsigned short general_flag;
	unsigned short compression;
	unsigned short lastmod_time;
	unsigned short lastmod_date;
	unsigned crc32;
	unsigned compressed_size;
	unsigned uncompressed_size;
	unsigned short filename_size;
	unsigned short extra_field_size;
	unsigned short file_comment_size;
	unsigned short disk_num;
	unsigned short internal_attr;
	unsigned external_attr;
	unsigned offset;
	std::string filename;
	std::string extra_field;
	std::string file_comment;
	CentralDirectoryEntry()
		: creator_version(0), min_version(0), general_flag(0), compression(0), lastmod_time(0),
		  lastmod_date(0), crc32(0), compressed_size(0), uncompressed_size(0), filename_size(0),
		  extra_field_size(0), file_comment_size(0), disk_num(0), internal_attr(0),
		  external_attr(0), offset(0), filename(), extra_field(), file_comment() {}
	~CentralDirectoryEntry() {}
};

struct CentralDirectoryEnd
{
	unsigned short disk_num;
	unsigned short cdir_disk;
	unsigned short disk_entries;
	unsigned short cdir_entries;
	unsigned cdir_size;
	unsigned cdir_offset;
	unsigned short comment_size;
	std::string comment;
	CentralDirectoryEnd()
		: disk_num(0), cdir_disk(0), disk_entries(0), cdir_entries(0),
		  cdir_size(0), cdir_offset(0), comment_size(0), comment() {}
	~CentralDirectoryEnd() {}
};

#define CDIR_ENTRY_SIG 0x02014b50
#define LOC_FILE_HEADER_SIG 0x04034b50
#define CDIR_END_SIG 0x06054b50

static unsigned char getByte(WPXInputStream *input)
{
	unsigned long numBytesRead = 0;
	const unsigned char *ret = input->read(1, numBytesRead);
	if (numBytesRead != 1)
		throw StreamException();
	return ret[0];
}

static unsigned short getShort(WPXInputStream *input)
{
	return (unsigned short)getByte(input)+((unsigned short)getByte(input)<<8);
}

static unsigned getInt(WPXInputStream *input)
{
	return (unsigned)getByte(input)+((unsigned)getByte(input)<<8)+((unsigned)getByte(input)<<16)+((unsigned)getByte(input)<<24);
}

static bool readCentralDirectoryEnd(WPXInputStream *input, CentralDirectoryEnd &end)
{
	try
	{
		unsigned signature = getInt(input);
		if (signature != CDIR_END_SIG)
			return false;

		end.disk_num = getShort(input);
		end.cdir_disk = getShort(input);
		end.disk_entries = getShort(input);
		end.cdir_entries = getShort(input);
		end.cdir_size = getInt(input);
		end.cdir_offset = getInt(input);
		end.comment_size = getShort(input);
		end.comment.clear();
		for (unsigned short i = 0; i < end.comment_size; i++)
			end.comment.append(1, (char)getByte(input));
	}
	catch (...)
	{
		return false;
	}
	return true;
}

static bool readCentralDirectoryEntry(WPXInputStream *input, CentralDirectoryEntry &entry)
{
	try
	{
		unsigned signature = getInt(input);
		if (signature != CDIR_ENTRY_SIG)
			return false;

		entry.creator_version = getShort(input);
		entry.min_version = getShort(input);
		entry.general_flag = getShort(input);
		entry.compression = getShort(input);
		entry.lastmod_time = getShort(input);
		entry.lastmod_date = getShort(input);
		entry.crc32 = getInt(input);
		entry.compressed_size = getInt(input);
		entry.uncompressed_size = getInt(input);
		entry.filename_size = getShort(input);
		entry.extra_field_size = getShort(input);
		entry.file_comment_size = getShort(input);
		entry.disk_num = getShort(input);
		entry.internal_attr = getShort(input);
		entry.external_attr = getInt(input);
		entry.offset = getInt(input);
		unsigned short i = 0;
		entry.filename.clear();
		for (i=0; i < entry.filename_size; i++)
			entry.filename.append(1, (char)getByte(input));
		entry.extra_field.clear();
		for (i=0; i < entry.extra_field_size; i++)
			entry.extra_field.append(1, (char)getByte(input));
		entry.file_comment.clear();
		for (i=0; i < entry.file_comment_size; i++)
			entry.file_comment.append(1, (char)getByte(input));
	}
	catch (...)
	{
		return false;
	}
	return true;
}

static bool readLocalFileHeader(WPXInputStream *input, LocalFileHeader &header)
{
	try
	{
		unsigned signature = getInt(input);
		if (signature != LOC_FILE_HEADER_SIG)
			return false;

		header.min_version = getShort(input);
		header.general_flag = getShort(input);
		header.compression = getShort(input);
		header.lastmod_time = getShort(input);
		header.lastmod_date = getShort(input);
		header.crc32 = getInt(input);
		header.compressed_size = getInt(input);
		header.uncompressed_size = getInt(input);
		header.filename_size = getShort(input);
		header.extra_field_size = getShort(input);
		unsigned short i = 0;
		header.filename.clear();
		for (i=0; i < header.filename_size; i++)
			header.filename.append(1, (char)getByte(input));
		header.extra_field.clear();
		for (i=0; i < header.extra_field_size; i++)
			header.extra_field.append(1, (char)getByte(input));
	}
	catch (...)
	{
		return false;
	}
	return true;
}

static bool areHeadersConsistent(const LocalFileHeader &header, const CentralDirectoryEntry &entry)
{
	if (header.min_version != entry.min_version)
		return false;
	if (header.general_flag != entry.general_flag)
		return false;
	if (header.compression != entry.compression)
		return false;
	if (!(header.general_flag & 0x08))
	{
		if (header.crc32 != entry.crc32)
			return false;
		if (header.compressed_size != entry.compressed_size)
			return false;
		if (header.uncompressed_size != entry.uncompressed_size)
			return false;
	}
	return true;
}

static bool findCentralDirectoryEnd(WPXInputStream *input)
{
	input->seek(0, WPX_SEEK_SET);
	try
	{
		while (!input->atEOS())
		{
			unsigned signature = getInt(input);
			if (signature == CDIR_END_SIG)
			{
				input->seek(-4, WPX_SEEK_CUR);
				return true;
			}
			else
				input->seek(-3, WPX_SEEK_CUR);
		}
	}
	catch (...)
	{
		return false;
	}
	return false;
}

static bool findDataStream(WPXInputStream *input, CentralDirectoryEntry &entry, const char *name)
{
	unsigned short name_size = strlen(name);
	if (!findCentralDirectoryEnd(input))
		return false;
	CentralDirectoryEnd end;
	if (!readCentralDirectoryEnd(input, end))
		return false;
	input->seek(end.cdir_offset, WPX_SEEK_SET);
	while (!input->atEOS() && (unsigned)input->tell() < end.cdir_offset + end.cdir_size)
	{
		if (!readCentralDirectoryEntry(input, entry))
			return false;
		if (name_size == entry.filename_size && entry.filename == name)
			break;
	}
	if (name_size != entry.filename_size)
		return false;
	if (entry.filename != name)
		return false;
	input->seek(entry.offset, WPX_SEEK_SET);
	LocalFileHeader header;
	printf("%x %x\n", (int)input->tell(), entry.offset);
	if (!readLocalFileHeader(input, header))
		return false;
	if (!areHeadersConsistent(header, entry))
		return false;
	return true;
}

} // anonymous namespace

bool WPXZipStream::isZipFile(WPXInputStream *input)
{
	// look for central directory end
	if (!findCentralDirectoryEnd(input))
		return false;
	CentralDirectoryEnd end;
	if (!readCentralDirectoryEnd(input, end))
		return false;
	input->seek(end.cdir_offset, WPX_SEEK_SET);
	// read first entry in the central directory
	CentralDirectoryEntry entry;
	if (!readCentralDirectoryEntry(input, entry))
		return false;
	input->seek(entry.offset, WPX_SEEK_SET);
	// read the local file header and compare with the central directory information
	LocalFileHeader header;
	if (!readLocalFileHeader(input, header))
		return false;
	if (!areHeadersConsistent(header, entry))
		return false;
	return true;
}

WPXInputStream *WPXZipStream::getSubstream(WPXInputStream *input, const char *name)
{
	CentralDirectoryEntry entry;
	if (!findDataStream(input, entry, name))
		return 0;
	unsigned long numBytesRead = 0;
	const unsigned char *compressedData = input->read(entry.compressed_size, numBytesRead);
	if (numBytesRead != entry.compressed_size)
		return 0;
	if (!entry.compression)
		return new WPXStringStream(compressedData, numBytesRead);
	else
	{
		int ret;
		z_stream strm;

		/* allocate inflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit2(&strm,-MAX_WBITS);
		if (ret != Z_OK)
			return 0;

		strm.avail_in = numBytesRead;
		strm.next_in = (Bytef *)compressedData;

		std::vector<unsigned char>data(entry.uncompressed_size);

		strm.avail_out = entry.uncompressed_size;
		strm.next_out = reinterpret_cast<Bytef *>(&data[0]);
		ret = inflate(&strm, Z_FINISH);
		switch (ret)
		{
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			data.clear();
			return 0;
		}
		return new WPXStringStream(&data[0], data.size());
	}
}

/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */