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

#include "FlacInfo.h"

#include "TagName.h"

CFlacInfo::CFlacInfo()
:	m_TagBlock(0),
	m_CuesheetFound(false)
{
}

CFlacInfo::~CFlacInfo()
{
	if (m_TagBlock)
		delete m_TagBlock;
}

bool CFlacInfo::CuesheetFound() const
{
	return m_CuesheetFound;
}

void CFlacInfo::SetFileName(const std::string& FileName)
{
	if (m_TagBlock)
		delete m_TagBlock;

	m_Tags.clear();
	m_Cuesheet.Clear();
			
	m_TagBlock=0;
	
	m_FileName=FileName;
	m_CuesheetFound=0;
}

bool CFlacInfo::Read()
{
	bool RetVal=false;

	if (!m_FileName.empty())
	{
		if (m_Chain.read(m_FileName.c_str()))
		{
			if (m_Chain.is_valid())
			{
				FLAC::Metadata::Iterator Iterator;
					
				Iterator.init(m_Chain);
				
				if (Iterator.is_valid())
				{
					do
					{
						switch (Iterator.get_block_type())
						{
							case FLAC__METADATA_TYPE_STREAMINFO:
								break;
								
							case FLAC__METADATA_TYPE_PADDING:
								break;
								
							case FLAC__METADATA_TYPE_APPLICATION:
								break;
								
							case FLAC__METADATA_TYPE_SEEKTABLE:
								break;
								
							case FLAC__METADATA_TYPE_VORBIS_COMMENT:
								m_TagBlock=(FLAC::Metadata::VorbisComment *)Iterator.get_block();
	
								for (unsigned count=0;count<m_TagBlock->get_num_comments();count++)
								{
									char Name[1024];
									char Value[1024];
									
									FLAC::Metadata::VorbisComment::Entry Entry=m_TagBlock->get_comment(count);
										
									strncpy(Name,Entry.get_field_name(),Entry.get_field_name_length());
									Name[Entry.get_field_name_length()]='\0';
	
									strncpy(Value,Entry.get_field_value(),Entry.get_field_value_length());
									Value[Entry.get_field_value_length()]='\0';
	
									m_Tags[CTagName(Name)]=Value;
								}
								
								break;
								
							case FLAC__METADATA_TYPE_CUESHEET:
							{
								m_CuesheetFound=true;
								
								FLAC::Metadata::CueSheet *Cuesheet=(FLAC::Metadata::CueSheet *)Iterator.get_block();
								for (unsigned count=0;count<Cuesheet->get_num_tracks();count++)
								{
									FLAC::Metadata::CueSheet::Track Track=Cuesheet->get_track(count);
										
									if (Track.get_number()==170)
										m_Cuesheet.SetLeadout(CalculateOffset(Track));
									else
										m_Cuesheet.AddTrack(CCuesheetTrack(Track.get_number(),CalculateOffset(Track)));
								}
								break;
							}
								
							case FLAC__METADATA_TYPE_UNDEFINED:  
								break;
						}
						
					} while (Iterator.next());
				}
			}
		}
	}
	
	return RetVal;
}

tTagMap CFlacInfo::Tags() const
{
	return m_Tags;
}

CCuesheet CFlacInfo::Cuesheet() const
{
	return m_Cuesheet;
}

int CFlacInfo::CalculateOffset(const FLAC::Metadata::CueSheet::Track& Track) const
{
	FLAC__uint64 Offset=Track.get_offset();
	FLAC__uint64 MaxIndexOffset=0;
	
	for (unsigned count=0;count<Track.get_num_indices();count++)
	{
		FLAC__StreamMetadata_CueSheet_Index Index=Track.get_index(count);
		
		if (Index.offset>MaxIndexOffset)
			MaxIndexOffset=Index.offset;
	}
	
	Offset+=MaxIndexOffset;
	Offset/=588;
	Offset+=150;
	
	return Offset;
}

bool CFlacInfo::WriteTags(const tTagMap& Tags)
{
	bool RetVal=false;
	
	if (m_TagBlock)
	{
		tTagMapConstIterator ThisTag=Tags.begin();
		
		while (Tags.end()!=ThisTag)
		{
			CTagName Name=(*ThisTag).first;
			std::string Value=(*ThisTag).second;

			if (!Value.empty())				
				SetTag(Name,Value);
			
			++ThisTag;
		}
		
		RetVal=m_Chain.write();
	}
	
	return RetVal;
}

void CFlacInfo::SetTag(const CTagName& Name, const std::string& Value)
{
	bool Found=false;
	bool Inserted=false;
	
	for (unsigned count=0;count<m_TagBlock->get_num_comments();count++)
	{
		FLAC::Metadata::VorbisComment::Entry Entry=m_TagBlock->get_comment(count);
		std::string ThisField=Entry.get_field_name();
			
		if (ThisField==Name.String() ||
			((ThisField=="DATE" || ThisField=="YEAR") && (Name.String()=="YEAR" || Name.String()=="DATE")))
		{
			Found=true;
		}
			
		if (Found)
		{
			if (!Inserted)
			{
				Inserted=true;
				Entry.set_field_name(Name.String().c_str());
				Entry.set_field_value(Value.c_str(),Value.length());
				m_TagBlock->set_comment(count,Entry);
			}
			else
			{
				m_TagBlock->delete_comment(count);
				count--;
			}
		}
	}
	
	if (!Found)
	{
		FLAC::Metadata::VorbisComment::Entry NewEntry;
			
		NewEntry.set_field_name(Name.String().c_str());
		NewEntry.set_field_value(Value.c_str(),Value.length());
		
		m_TagBlock->insert_comment(m_TagBlock->get_num_comments(),NewEntry);
	}
}
