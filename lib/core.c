#include <core.h>

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
  int i = 0, e = 0;
  
  while(doc_str[i])
  {
    while(doc_str[i] == ' ') i++;
    
    e = i;
    while(doc_str[e] != ' ') e++;
    
    matchWord(&doc_str[i], e - i);
        
    i += e;
  }
    
}

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
   
    return EC_SUCCESS;
}
