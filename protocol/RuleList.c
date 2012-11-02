
#include "PositionUpdateRule.h"
#include "AppLayer.h"
#include "myList.h"
#include <string.h>


void Rule_Add(struct list_head *new, struct list_head *head)
{
	list_add(new,head);
}

void Rule_Del(struct list_head *entry)
{
	list_del(entry);
}

// search for the id rule 
// return : 1 --> find
int Rule_Search(uint8_t *id, struct list_head *phead, Rule_Struct **Rule_node)
{
	struct list_head *plist;
	Rule_Struct *pRule;
	
	list_for_each(plist, phead) {
		pRule = list_entry(plist, Rule_Struct, list);
	
		if(0 == strcmp((const char *)id,(const char *)&(pRule->ID[0]))){
			*Rule_node = pRule;
			break;
		}
	}
	
	if(plist != phead) 
		return 1;		// we get the rule
	else
		return 0;		// rule not find
}


void Rule_Del_All(struct list_head *phead)
{
	struct list_head *plist;
	Rule_Struct *pRule;
	
	list_for_each(plist, phead) {
		pRule = list_entry(plist, Rule_Struct, list);
		Rule_Del(&(pRule->list));
		free(pRule);
	}
}
