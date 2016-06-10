/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "utils.hxx"

#include "avtk.hxx"
#include "theme.hxx"
#include <algorithm>
#include <cstring>
#include <sndfile.h>

namespace Avtk
{
int loadSample( std::string path, std::vector< float >& sample, bool printErrors )
{
#ifdef AVTK_SNDFILE
	SF_INFO info;
	memset( &info, 0, sizeof( SF_INFO ) );
	SNDFILE* const sndfile = sf_open( path.c_str(), SFM_READ, &info);
	if ( !sndfile ) {
		printf("Failed to open sample '%s'\n", path.c_str() );
		return -1;
	}

	if( !(info.channels == 1 || info.channels == 2) ) {
		int chnls = info.channels;
		printf("Loading sample %s, channels = %i\n", path.c_str(), chnls );
		return -1;
	}

	sample.resize( info.frames * info.channels );

	sf_seek(sndfile, 0ul, SEEK_SET);
	sf_read_float( sndfile, &sample[0], info.frames * info.channels );
	sf_close(sndfile);

	return OPENAV_OK;
#else
	if( printErrors ) {
		printf("AVTK compiled without SNDFILE support: cannot load audio sample.\n");
	}
	return OPENAV_ERROR;
#endif
}

} //  Avtk namespace
