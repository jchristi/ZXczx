/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#include "stdafx.h"
#include "TA3D_NameSpace.h"			// our namespace, a MUST have.
#include "ta3dbase.h"
#include "misc/math.h"
#include "misc/paths.h"
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;


namespace TA3D
{

void install_TA_files( String HPI_file, String filename )
{
	uint32 file_size32 = 0;
	SmartPtr<Archive> archive = Archive::load(HPI_file);
	if (!archive)
    {
        LOG_ERROR("archive not found : '" << HPI_file << "'");
        return;
    }
	std::deque<Archive::File*> lFiles;
    archive->getFileList(lFiles);
	byte *data = archive->readFile(filename, &file_size32);			// Extract the file
	if (data)
	{
		Stream dst(Paths::Resources + Paths::ExtractFileName(filename), OpenMode::write);

		if (dst.opened())
		{
			dst.write((const char*)data, file_size32);

			dst.flush();
			dst.close();
		}
		DELETE_ARRAY(data);
	}
}

} // namespace TA3D
