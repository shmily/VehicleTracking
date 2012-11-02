/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:23
#      Filename : Btype_Control.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __BTYPE_CONTROL_H_
#define __BTYPE_CONTROL_H_

// error code define
#define		_Btype_Tx_Success		0x00
#define		_Btype_Tx_Control_ERR	0x01
#define		_Btype_Tx_Empty			0x02
#define		_Btype_ID_ERR			0x03
#define		_Btype_COM_ERR			0x04


typedef enum _Btype_State_Struct_
{  
	Btype_Idle,
	
	Btype_REQ_SEND,
	Btype_REQ_ACK_OK,
	Btype_REQ_ACK_ERR,

	Btype_Packet_SEND,
	Btype_Packet_ACK_OK,
	Btype_Packet_ACK_ERR,

	Btype_HAL_ERR,
	Btype_OVER
}Btype_State;


Btype_State Btype_GetCurrentState(void);
void		Btype_SetState(Btype_State state);
int 		ReportUpdata_Init(const void *pdata_src, int len);
int 		ReportUpdata_Loop(int flag);

#endif