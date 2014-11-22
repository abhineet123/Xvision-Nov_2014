#ifndef _xv_dvframe_h
#define _xv_dvframe_h
/*
 * frame.h -- utilities for process digital video frames
 * Copyright (C) 2000 Arne Schirmacher <arne@schirmacher.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

class DVFrame
{

        class Pack
        {
           public:
                unsigned char	data[5];
        };
public:
        unsigned char	*data;
        int	bytesInDVFrame;

        DVFrame();
        ~DVFrame();

        void GetSSYBPack(int packNum, Pack &pack);
        void GetVAUXPack(int packNum, Pack &pack);
        void GetAAUXPack(int packNum, Pack &pack);
        void GetTimeCode(char *s);
        void GetRecordingDate(char *s);
        bool IsPAL(void) {return (data[3] & 0x80)!=0;};
        bool is61834(void) {return (data[3] & 0x1)!=0;};
        bool IsNewRecording(void);
        bool IsComplete(void);
};
#endif
