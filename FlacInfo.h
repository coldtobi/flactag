/* --------------------------------------------------------------------------

   flactag -- A tagger for single album FLAC files with embedded CUE sheets
   						using data retrieved from the MusicBrainz service

   Copyright (C) 2006-2012 Andrew Hawkins
   Copyright (C) 2011-2012 Daniel Pocock

   This file is part of flactag.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

   Flactag is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

     $Id$

----------------------------------------------------------------------------*/

#ifndef _FLAC_INFO_H
#define _FLAC_INFO_H

#include <string>

#include <FLAC++/metadata.h>

#include "TagName.h"
#include "Cuesheet.h"
#include "CoverArt.h"
#include "WriteInfo.h"

class CFlacInfo
{
public:
	CFlacInfo();
	~CFlacInfo();

	void SetFileName(const std::string& FileName);
	bool Read();
	tTagMap Tags() const;
	CCoverArt CoverArt() const;
	CCuesheet Cuesheet() const;
	bool CuesheetFound() const;
	std::string WriteError() const;

	bool WriteInfo(const CWriteInfo& WriteInfo);
	void SetTag(const CTagName& Name, const std::string& Value);

private:
	int CalculateOffset(const FLAC::Metadata::CueSheet::Track& Track) const;
	void SetPictureBlock(const CCoverArt& CoverArt);
	bool UpdateCuesheet();

	std::string m_FileName;
	tTagMap m_Tags;
	CCuesheet m_Cuesheet;
	FLAC::Metadata::Chain m_Chain;
	FLAC::Metadata::VorbisComment *m_TagBlock;
	FLAC::Metadata::Picture *m_PictureBlock;
	bool m_CuesheetFound;
	CCoverArt m_CoverArt;
	std::string m_WriteError;
};

#endif
