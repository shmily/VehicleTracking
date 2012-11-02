/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-11-02 10:19
#      Filename : DrowsyDetect.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _DROWSY_DETECT_H_
#define _DROWSY_DETECT_H_

typedef struct _DrowsyImage_Info_
{
	char image_path[64];
	char image_protocol_name[6];
	int image_send;				// image_send = 1 --> need to send
}DrowsyImage_Info;

int Open_ImageFIFO(const char *path);
int DrowsyImage_Check(void);
int DrowsyImage_Send(void);

#endif

