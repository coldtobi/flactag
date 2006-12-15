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

#include "CommandLine.h"

#include <sstream>

#include <getopt.h>

#include "ErrorLog.h"

CCommandLine::CCommandLine(int argc, char *const argv[])
:	m_Valid(true),
	m_Check(false),
	m_Write(false),
	m_Rename(false),
	m_ForceMulti(false),
	m_Version(false),
	m_DiscID(false),
	m_ForceWrite(false)
{
	struct option LongOptions[] =
	{
		{"force-write", no_argument, 0, 'f'},
		{"discid", no_argument, 0, 'd'},
		{"version", no_argument, 0, 'v'},
		{"force-multi", no_argument, 0, 'm'},
		{"rename", no_argument, 0, 'r'},
		{"write", no_argument, 0, 'w'},
		{"check", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};
             
	int OptionIndex=0;
	int Ret;

	opterr=1;
		
	do
	{
		Ret=getopt_long(argc,argv,"fdvmrwc",LongOptions,&OptionIndex);
		switch (Ret)
		{
			case 'f':
				m_ForceWrite=true;
				break;
				
			case 'd':
				m_DiscID=true;
				break;
				
			case 'v':
				m_Version=true;
				break;
				
			case 'm':
				m_ForceMulti=true;
				break;
				
			case 'r':
				m_Rename=true;
				break;
				
			case 'w':
				m_Write=true;
				break;
				
			case 'c':
				m_Check=true;
				break;

			case -1:
				//Reached end of options
				break;
				
			default:
				m_Valid=false;
				break;
		}
	} while (m_Valid && -1!=Ret);

	if (m_Valid)
	{	
		int LastArg=optind;
		
		while (LastArg!=argc)
		{
			m_FileNames.push_back(argv[LastArg]);
			LastArg++;
		}
	}
	
	if (m_FileNames.empty() && !m_Version)
		m_Valid=false;
	else if (m_DiscID && (m_Check || m_Write || m_Rename || m_ForceMulti || m_ForceWrite))
		m_Valid=false;
	else if (m_ForceWrite && !m_Write)
		m_Valid=false;
	
	if (!m_Valid)
		Usage(argv[0]);
}

bool CCommandLine::Valid() const
{
	return m_Valid;
}

bool CCommandLine::Check() const
{
	return m_Check;
}

bool CCommandLine::Write() const
{
	return m_Write;
}

bool CCommandLine::Rename() const
{
	return m_Rename;
}

bool CCommandLine::ForceMulti() const
{
	return m_ForceMulti;
}

bool CCommandLine::Version() const
{
	return m_Version;
}

bool CCommandLine::DiscID() const
{
	return m_DiscID;
}

bool CCommandLine::ForceWrite() const
{
	return m_ForceWrite;
}

std::vector<std::string> CCommandLine::FileNames() const
{
	return m_FileNames;
}

void CCommandLine::Usage(const std::string& ProgName) const
{
	printf("Usage: %s [ --version | -v ] [ --discid | -d] [ --check | -c ]\n"
					"\t\t[ --write | -w ] [ --rename | -r ]\n"
					"\t\t[ --force-multi | -m ] [ --force-write | -f ]\n"
					"\t\tflacfile [ flacfile ] [ flacfile ]\n",ProgName.c_str());
}
