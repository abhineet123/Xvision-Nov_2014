/*
 * frame.cc -- utilities for processing digital video frames
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

#include "DVFrame.h"
#include <stdio.h>
#include <string.h>

DVFrame::DVFrame() : bytesInDVFrame(0)
{
    data=new unsigned char[144000];
    memset(data, 0, 144000);
}


DVFrame::~DVFrame()
{ delete data;}


/* get a packet from the subcode data */

void DVFrame::GetSSYBPack(int packNum, Pack &pack)
{
        int seqCount = (data[3] & 0x80)? 12 : 10;

        for (int i = 0; i < seqCount; ++i) {
                for (int j = 0; j < 2; ++j) {
                        for (int k = 0; k < 6; ++k) {
                               unsigned char *s = &data[i * 150 * 80 + 1 * 80 + j * 80 + 3 + k * 8 + 3];
                                if (s[0] == packNum) {
                                        pack.data[0] = s[0];
                                        pack.data[1] = s[1];
                                        pack.data[2] = s[2];
                                        pack.data[3] = s[3];
                                        pack.data[4] = s[4];
                                        return ;
                                }
                        }
                }
        }
}


/* get a packet from the video auxiliary data */

void DVFrame::GetVAUXPack(int packNum, Pack &pack)
{
        int seqCount = (data[3] & 0x80)? 12 : 10;

        for (int i = 0; i < seqCount; ++i) {
                for (int j = 0; j < 3; ++j) {
                        for (int k = 0; k < 15; ++k) {
                                unsigned char *s = &data[i * 150 * 80 + 3 * 80 + j * 80 + 3 + k * 5];
                                if (s[0] == packNum) {
                                        pack.data[0] = s[0];
                                        pack.data[1] = s[1];
                                        pack.data[2] = s[2];
                                        pack.data[3] = s[3];
                                        pack.data[4] = s[4];
                                        return ;
                                }

                        }
                }
        }
}


/* get a packet fromthe audio auxiliary data */

void DVFrame::GetAAUXPack(int packNum, Pack &pack)
{
        int seqCount = (data[3] & 0x80)? 12 : 10;

        for (int i = 0; i < seqCount; ++i) {
                for (int j = 0; j < 9; ++j) {
                        unsigned char *s = &data[i * 150 * 80 + 6 * 80 + j * 16 * 80 + 3];
                        if (s[0] == packNum) {
                                pack.data[0] = s[0];
                                pack.data[1] = s[1];
                                pack.data[2] = s[2];
                                pack.data[3] = s[3];
                                pack.data[4] = s[4];
                                return ;
                        }
                }
        }
}


/* check the flag for start of new recording in the audio data */

bool DVFrame::IsNewRecording()
{
        Pack aauxSourceControl;

        GetAAUXPack(0x51, aauxSourceControl);
	return !(aauxSourceControl.data[2] & 0x80);
}


/* check whether we have received as many bytes as expected for this frame */

bool DVFrame::IsComplete(void)
{

        if (bytesInDVFrame == 0)
                return false;
        return (data[3]&0x80)?  bytesInDVFrame == 144000:
		  		bytesInDVFrame == 120000;
}


