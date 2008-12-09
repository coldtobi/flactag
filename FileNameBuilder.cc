/* --------------------------------------------------------------------------

   flactag -- A tagger for single album FLAC files with embedded CUE sheets
   						using data retrieved from the MusicBrainz service

   Copyright (C) 2006 Andrew Hawkins
   
   This file is part of flactag.
   
   Flactag is free software; you can redistribute it and/or
   modify it under the terms of v2 of the GNU Lesser General Public
   License as published by the Free Software Foundation.
   
   Flactag is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

     $Id$

----------------------------------------------------------------------------*/

#include "FileNameBuilder.h"

#include <stdlib.h>

#include <unac.h>

#include "ErrorLog.h"

CFileNameBuilder::CFileNameBuilder(const tTagMap& Tags, const std::string& BasePath, const std::string& SingleDiskFileName, const std::string& MultiDiskFileName)
:	m_Tags(Tags),
	m_BasePath(BasePath),
	m_SingleDiskFileName(SingleDiskFileName),
	m_MultiDiskFileName(MultiDiskFileName)
{
	BuildPath();
}
	
std::string CFileNameBuilder::FileName() const
{
	return m_FileName;
}

void CFileNameBuilder::BuildPath()
{
	std::string Template;
	
	if (m_Tags[CTagName("DISCNUMBER")].empty())
		Template=m_SingleDiskFileName;
	else
		Template=m_MultiDiskFileName;
		
	m_FileName=m_BasePath;
	if (m_FileName[m_FileName.length()-1]!='/')
		m_FileName+="/";
		
	m_FileName+=Template;
	
	ReplaceString("%A","ARTIST",false);
	ReplaceString("%S","ARTISTSORT",false);
	ReplaceString("%T","ALBUM",false);
	ReplaceString("%D","DISCNUMBER",false);
	ReplaceString("%Y","YEAR",false);
	ReplaceString("%G","GENRE",false);
	ReplaceString("%1","ARTISTSORT",true);
}

void CFileNameBuilder::ReplaceString(const std::string& Search, const std::string& ReplaceTag, bool FirstOnly)
{
	std::string Replace="NO"+ReplaceTag;
	
	tTagMapConstIterator ThisTag=m_Tags.find(CTagName(ReplaceTag));
	if (m_Tags.end()!=ThisTag)
	{
		CUTF8Tag Value=(*ThisTag).second;
		if (!Value.empty())
			Replace=FixString(Value.UTF8Value());
	}

	if (FirstOnly)
	{
		bool Found=false;
		
		for (std::string::size_type count=0;count<Replace.length();count++)
		{
			if (isalnum(Replace[count]))
			{
				Replace=std::toupper(Replace[count]);
				Found=true;
				break;
			}
		}
		
		if (!Found)
			Replace="extended";
	}
			
	std::string::size_type SearchPos=m_FileName.find(Search);
	while(std::string::npos!=SearchPos)
	{
		m_FileName=m_FileName.substr(0,SearchPos)+Replace+m_FileName.substr(SearchPos+Search.length());
		SearchPos=m_FileName.find(Search);
	}
}

std::string CFileNameBuilder::FixString(const std::string& String) const
{
	char *out=0;
	size_t out_length=0;
	std::string Fixed=String;
		
	if (0==unac_string("UTF-8",String.c_str(),String.length(),&out,&out_length))
		Fixed=out;
	else
		perror("unac_string");

	if (out)
		free(out);
				
	std::string BadChars="/:\"'`;?&,.!";
		
	for (std::string::size_type count=0;count<Fixed.length();count++)
	{
		if (BadChars.find(Fixed[count])!=std::string::npos)
			Fixed.replace(count,1,"-");
	}

	return Fixed;
}
