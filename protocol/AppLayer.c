
#include "AppLayer.h"
#include "DeviceControl.h"
#include "PositionUpdateRule.h"
#include "ReportCondition.h"
#include "ObjectRequest.h"

void InputCommandHandle(PacketInfo *APP_Packet)
{
	CommandType Command;
	uint8_t		*pdata;
	uint16_t	tmp;

	DEBUG("%s\n",__func__);
	
	pdata = APP_Packet->DatPointer;
	
	Command.Type = *(pdata);
	tmp = *(pdata+1);
	Command.Action = (tmp<<8) + *(pdata+2);
	Command.pData = (pdata+3);
	Command.length = (APP_Packet->length - 3);	
	Command.plowLevelInfo = APP_Packet;
	
	// device control handle
	if(Command.Type == DEV_CONTROL){
		DEBUG("%s : This is a DEV_CONTROL packet...\n",__func__);
		DeviceControlHandle(&Command);
	}
	
	// position update rule handle
	if((Command.Type >= RULE_SET)&&(Command.Type <= RULE_SWITCH)){
		DEBUG("%s : This is a position update rule packet...\n",__func__);
		PositionUpdateRuleHandle(&Command);
	}

	// position update rule handle
	if(Command.Type == REPORT_SET){
		DEBUG("%s : This is a report condition set packet...\n",__func__);
		ReportConditionHandle(&Command);
	}
	
	// ObjectRequest packet
	if(Command.Type == HEART_BEAT){
		DEBUG("%s : This is a Object Request packet...\n",__func__);
		ObjectRequestHandle(&Command);
	}
}
