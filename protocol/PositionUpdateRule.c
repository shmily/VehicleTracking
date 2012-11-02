
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "PositionUpdateRule.h"
#include "myList.h"
#include "RuleList.h"

#include "UDP_Lib.h"
#include "crc16.h"

struct list_head *pRule_List;
Rule_ReqACK_info ACK_buf;
Rule_ReqACK_info Rule_Del_ACK_buf;


// static functions			  
static APP_Error SetRule_Request(CommandType *command);
static APP_Error DelRule_Request(CommandType *command);

static void SimpleAKV_Parser( uint8_t  *pdata_start, 
							  uint8_t  *pAttr, 
							  uint8_t  *pK_len, 
							  uint16_t *pV_len, 
							  uint8_t  **pKey, 
							  uint8_t  **Value);

static void * malloc_RuleKeyValue(uint8_t KeyType, uint8_t *pdata);
static void KeyValue_Printf(uint8_t type, void *pdata);
static void Rule_Printf(Rule_Struct *pRule);

static APP_Error SetRule_Request_ACK(CommandType *command, Rule_ReqACK_info *pACK_buf);
static APP_Error DelRule_Request_ACK(CommandType *command, Rule_ReqACK_info *pACK_buf);


void PositionUpdateRule_Initial(struct list_head *phead)
{
	INIT_LIST_HEAD(phead);
	pRule_List = phead;
}

APP_Error PositionUpdateRuleHandle(CommandType *command)
{
	APP_Error res;
	
	// set rule request
	if(command->Type == RULE_SET){
		DEBUG("%s : This is a set rule request packet...\n",__func__);
		res = SetRule_Request(command);
		
		if(res==APP_ERR_NONE) {
			res = SetRule_Request_ACK(command, &ACK_buf);
		}
			
	} else if(command->Type == RULE_DEL){
		DEBUG("%s : This is a del rule request packet...\n",__func__);
		res = DelRule_Request(command);
		
		if(res==APP_ERR_NONE){
			res = DelRule_Request_ACK(command, &Rule_Del_ACK_buf);
		}
	}
	
	return res;
}


static APP_Error SetRule_Request(CommandType *command)
{
	uint8_t 	*pdata;				// ponit to the data buff
	uint16_t 	tmp;
	uint8_t 	i;			
	uint8_t 	id_len;				// id length
	uint8_t 	id_tmp[_ID_LEN];	// buff the id
	uint8_t 	Need_Confirm;		// if the packet ack need to add the rule ack 	
	Rule_Struct *pRule;
	
	// for AKV parser
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;
	
	pdata = command->pData;
	
	// get the rule id
	id_len = *(pdata++);
	DEBUG("%s : [id_len = %d]\n",__func__,id_len);
	assert(id_len <= _ID_LEN);		// we assert that the id_len < _ID_LEN
	memset(id_tmp,0,_ID_LEN);		// clear the id str
	for(i=0;i<id_len;i++){
		id_tmp[i] = *(pdata++);
	}
	DEBUG("%s : [ID : %s]\n",__func__,id_tmp);

	if(Rule_Search(id_tmp, pRule_List, &pRule) != 1){	// new rule id
		pRule = malloc(sizeof(Rule_Struct));			// malloc ram for the rule 
		memset(pRule, 0, sizeof(Rule_Struct));			// clear the rule
		Rule_Add(&(pRule->list),pRule_List);
	} else {											// we have the rule, then we need to free the prv memery
		free(pRule->BMAP);
		free(pRule->TMAP);
		free(pRule->EMAP);
	}
	
	// save the id
	memset(pRule->ID,0,_ID_LEN);
	ACK_buf.id_length = id_len;							// !!we save this for ack
	
	for(i=0;i<id_len;i++){
		pRule->ID[i]  = id_tmp[i];
		ACK_buf.id[i] = id_tmp[i];						// !!we save this for ack
	}
	
	// if the packet ack need to add the rule ack 
	Need_Confirm = *(pdata++);
	DEBUG("%s : Need_Confirm : [%d]\n",__func__,Need_Confirm);
	ACK_buf.Ack = 0x01;									// !!we save this for ack
	
	// save the AreMove
	pRule->Effective.AreMove = *(pdata++);
	ACK_buf.AreMove = pRule->Effective.AreMove;			// !!we save this for ack
	
	// save the cnt
	tmp = *(pdata++);
	tmp = (tmp<<8) + (*(pdata++));
	pRule->Effective.Cnt = tmp;
	ACK_buf.Cnt = pRule->Effective.Cnt;					// !!we save this for ack

	// save the Interval
	tmp = *(pdata++);
	tmp = (tmp<<8) + (*(pdata++));
	pRule->Effective.Interval = tmp;
	ACK_buf.Interval = pRule->Effective.Interval;		// !!we save this for ack

		
	DEBUG("%s : Paser the BMAP...\n",__func__);
	SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);		// save the BMP rule
	assert(Attr == 0);
	assert(K_len == 1);
	pRule->BMAP_Type = (*pKey);
	pRule->BMAP = malloc_RuleKeyValue(pRule->BMAP_Type, pValue);
	
	ACK_buf.bm_length = (pValue + V_len) - pdata;
	memcpy(ACK_buf.bm, pdata, ACK_buf.bm_length);						// !!we save this for ack
	
	DEBUG("%s : Paser the TMAP...\n",__func__);
	pdata = pValue + V_len;												// ponit to the TMP
	SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);		// save the TMP rule
	assert(Attr == 0);
	assert(K_len == 1);
	pRule->TMAP_Type = (*pKey);
	pRule->TMAP = malloc_RuleKeyValue(pRule->TMAP_Type, pValue);

	ACK_buf.tm_length = (pValue + V_len) - pdata;
	memcpy(ACK_buf.tm, pdata, ACK_buf.tm_length);						// !!we save this for ack
	
	DEBUG("%s : Paser the EMAP...\n",__func__);
	pdata = pValue + V_len;												// ponit to the EMP
	SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);		// save the EMP rule
	assert(Attr == 0);
	assert(K_len == 1);
	pRule->EMAP_Type = (*pKey);
	pRule->EMAP = malloc_RuleKeyValue(pRule->EMAP_Type, pValue);

	ACK_buf.em_length = (pValue + V_len) - pdata;
	memcpy(ACK_buf.em, pdata, ACK_buf.em_length);						// !!we save this for ack
	
	return APP_ERR_NONE;
}


static APP_Error SetRule_Request_ACK(CommandType *command, Rule_ReqACK_info *pACK_buf)
{
	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	
	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);
	
	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = _TransmitCtrl;			// device to server
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	//need to change
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = command->plowLevelInfo->SEQ_num;
	
	// send the ack
	*(pData++) = RULE_SET;							// command type
	*(pData++) = 0x00;
	*(pData++) = 0x00;								// action
	
	// I_len
	*(pData++) = pACK_buf->id_length;
	
	// id
	memcpy(pData, pACK_buf->id, pACK_buf->id_length);
	pData = pData + pACK_buf->id_length;

	// Ack
	*(pData++) = pACK_buf->Ack;						// success : 0x01; faild : 0x00
	
	/*
	// --- ar attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 'a';
	*(pData++) = 'r';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x01;
	// value
	*(pData++) = pACK_buf->AreMove;
	
	// --- ts attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 't';
	*(pData++) = 's';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x02;
	// value
	*(pData++) = (uint8_t)((pACK_buf->Cnt)>>8);	
	*(pData++) = (uint8_t)(pACK_buf->Cnt);	
	
	// --- i attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x01;
	// key
	*(pData++) = 'i';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x02;
	// value
	*(pData++) = (uint8_t)((pACK_buf->Interval)>>8);	
	*(pData++) = (uint8_t)(pACK_buf->Interval);	
	
	// --- bm attr ---
	// attr
	*(pData++) = 0x01;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 'b';
	*(pData++) = 'M';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = pACK_buf->bm_length;
	// value
	memcpy(pData, pACK_buf->bm, pACK_buf->bm_length);
	pData = pData + pACK_buf->bm_length;

	// --- tm attr ---
	// attr
	*(pData++) = 0x01;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 't';
	*(pData++) = 'M';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = pACK_buf->tm_length;
	// value
	memcpy(pData, pACK_buf->tm, pACK_buf->tm_length);
	pData = pData + pACK_buf->tm_length;

	// --- tm attr ---
	// attr
	*(pData++) = 0x01;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 'e';
	*(pData++) = 'M';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = pACK_buf->em_length;
	// value
	memcpy(pData, pACK_buf->em, pACK_buf->em_length);
	pData = pData + pACK_buf->em_length;
	*/
	
	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);

	int i;

	for(i=0;i<Packet.length;i++){
		fprintf(stderr,"0x%02X ", Packet.Data[i]);
	}


	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : ACK ...OK\n",__func__);
		return APP_ERR_NONE;
	}
	
}


static APP_Error DelRule_Request(CommandType *command)
{
	uint8_t 	*pdata;				// ponit to the data buff
	uint8_t 	i;			
	uint8_t 	id_len;				// id length
	uint8_t 	id_tmp[_ID_LEN];	// buff the id
	Rule_Struct *pRule;

	pdata = command->pData;
	
	// get the rule id
	id_len = *(pdata++);
	DEBUG("%s : [id_len = %d]\n",__func__,id_len);
	assert(id_len <= _ID_LEN);							// we assert that the id_len < _ID_LEN
	memset(id_tmp,0,_ID_LEN);							// clear the id str
	for(i=0;i<id_len;i++){
		id_tmp[i] = *(pdata++);
	}
	DEBUG("%s : [ID : %s]\n",__func__,id_tmp);
	
	Rule_Del_ACK_buf.id_length = id_len;				// !!we save this for ack
	memcpy(Rule_Del_ACK_buf.id, id_tmp, id_len);
	
	if((id_len==1)&&(id_tmp[0]=='*')){							// remove all rules
		Rule_Del_All(pRule_List);
		Rule_Del_ACK_buf.Ack = 0x01;
	} else if(Rule_Search(id_tmp, pRule_List, &pRule) == 1) {	// if we find the rule, del it
		Rule_Del(&(pRule->list));
		free(pRule);
		Rule_Del_ACK_buf.Ack = 0x01;
	} else {
		fprintf(stderr,"**warning: the rule \"%s\" does not exist!\n",id_tmp);
		Rule_Del_ACK_buf.Ack = 0x00;
	}
	
	return APP_ERR_NONE;
}


static APP_Error DelRule_Request_ACK(CommandType *command, Rule_ReqACK_info *pACK_buf)
{
	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	
	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);
	
	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = _TransmitCtrl;			// device to server
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	//need to change
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = command->plowLevelInfo->SEQ_num;
	
	// send the ack
	*(pData++) = RULE_DEL;							// command type
	*(pData++) = 0x00;
	*(pData++) = 0x00;								// action
	
	// id_length
	*(pData++) = pACK_buf->id_length;
	
	//id
	memcpy(pData, pACK_buf->id, pACK_buf->id_length);
	pData = pData + pACK_buf->id_length;
	
	// Ack
	*(pData++) = pACK_buf->Ack;						// success : 0x01; faild : 0x00
	
	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);
	
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : ACK ...OK\n",__func__);
		return APP_ERR_NONE;
	}
	
	return APP_ERR_NONE;
}


static void SimpleAKV_Parser(uint8_t *pdata_start, uint8_t *pAttr, uint8_t *pK_len, uint16_t *pV_len, uint8_t **pKey, uint8_t **pValue)
{
	uint8_t  *pdata;
	uint16_t tmp;
	
	pdata = pdata_start;
	
	(*pAttr)  =  *(pdata++);
	(*pK_len) =  *(pdata++);
	
	(*pKey) = pdata;			// save the key pointer
	
	pdata = pdata + (*pK_len);	// point to the V_len
	tmp = *(pdata++);
	tmp = (tmp<<8) + (*(pdata++));
	(*pV_len) = tmp;
	
	(*pValue) = pdata;			// save the Value pointer
}

static void * malloc_RuleKeyValue(uint8_t KeyType, uint8_t *pdata)
{
	void 		*p;
	uint32_t 	*puint32_t;
	uint16_t	*puint16_t;
	uint32_t 	tmp;
	Time_DT		*pTime;
	
	assert((KeyType>='1')&&(KeyType<='9'));
	
	switch (KeyType){
		case '1' :
		case '8' :
		case '9' :
					p = malloc(sizeof(uint32_t));		// malloc the ram
					memset(p, 0, sizeof(uint32_t));
					tmp = *(pdata++);					// save the value
					tmp = (tmp << 8) + *(pdata++);
					tmp = (tmp << 8) + *(pdata++);
					tmp = (tmp << 8) + *(pdata++);
					puint32_t = (uint32_t *)p;
					*(puint32_t) = tmp;		
					break;
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '7' :
					p = malloc(sizeof(uint16_t));		// malloc the ram
					memset(p, 0, sizeof(uint16_t));
					tmp = *(pdata++);					// save the value
					tmp = (tmp << 8) + *(pdata++);
					puint16_t = (uint16_t *)p;			
					*(puint16_t) = tmp;
					break;
		case '6' :
					p = malloc(sizeof(Time_DT));
					memset(p, 0, sizeof(Time_DT));
					pTime = (Time_DT *)p;
					pTime->year   = *(pdata++);
					pTime->month  = *(pdata++);
					pTime->day    = *(pdata++);
					pTime->hour   = *(pdata++);
					pTime->minute = *(pdata++);
					pTime->second = *(pdata++);
					break;
		default :	
					p = malloc(sizeof(Time_DT));
					memset(p, 0, sizeof(Time_DT));
					break;
	}
	
	return p;
}

void ShowRules(struct list_head *phead)
{
	struct list_head *plist;
	Rule_Struct *pRule;
	
	list_for_each(plist, phead) {
		pRule = list_entry(plist, Rule_Struct, list);
		
		Rule_Printf(pRule);
		fprintf(stderr,"=========================\n");
	}
}

static void Rule_Printf(Rule_Struct *pRule)
{
	fprintf(stderr,"Rule ID       : %s\n", pRule->ID);
	fprintf(stderr,"Rule AreMove  : %d\n", pRule->Effective.AreMove);
	fprintf(stderr,"Rule Cnt      : %d\n", pRule->Effective.Cnt);
	fprintf(stderr,"Rule Interval : %d s\n", pRule->Effective.Interval);
	
	//BMP
	fprintf(stderr,"--- BMP Rule ---\n");
	KeyValue_Printf(pRule->BMAP_Type, pRule->BMAP);
	
	//TMP
	fprintf(stderr,"--- TMP Rule ---\n");
	KeyValue_Printf(pRule->TMAP_Type, pRule->TMAP);

	//EMP
	fprintf(stderr,"--- EMP Rule ---\n");
	KeyValue_Printf(pRule->EMAP_Type, pRule->EMAP);
	
}

static void KeyValue_Printf(uint8_t type, void *pdata)
{
	uint16_t *pValue16;
	uint32_t *pValue32;
	Time_DT  *pValueDT;
	
	fprintf(stderr,"BMP Rule Key  : %d\n", type-0x30);
	switch (type){
		case '1' :
		case '8' :
		case '9' :
					pValue32 = (uint32_t *)pdata;
					fprintf(stderr,"Value         : %d\n", (*pValue32));
					break;
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '7' :
					pValue16 = (uint16_t *)pdata;
					fprintf(stderr,"Value         : %d\n", (*pValue16));
					break;
		case '6' :
					pValueDT = (Time_DT  *)pdata;
					fprintf(stderr,"Value         : %02d-%02d-%02d\n", pValueDT->year,pValueDT->month,pValueDT->day);
					fprintf(stderr,"                %02d:%02d:%02d\n", pValueDT->hour,pValueDT->minute,pValueDT->second);
					break;
		default :	
					break;
	}
}


