
#include "ReportCondition.h"
#include "AppLayer.h"
#include "myList.h"
#include <string.h>

void ReportCondition_Add(struct list_head *new, struct list_head *head)
{
	list_add(new,head);
}

void ReportCondition_Del(struct list_head *entry)
{
	list_del(entry);
}

// search for the Report condition
// return : 1 --> find
int ReportCondition_Search(uint32_t index, struct list_head *phead, InZone_Report_Struct **Condition_node)
{
	struct list_head 		*plist;
	InZone_Report_Struct 	*pCondition;
	
	list_for_each(plist, phead) {
		pCondition = list_entry(plist, InZone_Report_Struct, list);
	
		if(index == pCondition->index){
			*Condition_node = pCondition;
			break;
		}
	}
	
	if(plist != phead) 
		return 1;		// we get the rule
	else
		return 0;		// rule not find
}

int TDSA_ReportCondition_Search(uint32_t index, struct list_head *phead, SpeedAnomaly_Report **Condition_node)
{
	struct list_head 		*plist;
	SpeedAnomaly_Report 	*pCondition;
	
	list_for_each(plist, phead) {
		pCondition = list_entry(plist, SpeedAnomaly_Report, list);
	
		if(index == pCondition->index){
			*Condition_node = pCondition;
			break;
		}
	}
	
	if(plist != phead) 
		return 1;		// we get the rule
	else
		return 0;		// rule not find
}

void ReportCondition_Del_All(struct list_head *phead)
{
/*	struct list_head *plist;
	Rule_Struct *pRule;
	
	list_for_each(plist, phead) {
		pRule = list_entry(plist, Rule_Struct, list);
		Rule_Del(&(pRule->list));
		free(pRule);
	}*/
}
