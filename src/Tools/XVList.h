// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVLIST_H_
#define _XVLIST_H_

/**
 * XVnode class
 * 
 * This is a simple node in a linked
 * list (XVList).  Each node in a given
 * list represent the images (XVImageBase)
 * that share the same pixmap (XVPixmap)
 */

class XVNode {

  public:

   XVNode * succ;

   XVNode() { succ = NULL; };
};

/** linked list class, 
 *  uses XVNodes as list elements
 */
class XVList {

 protected:
  
  int		count;
  XVNode	*head;
  
 public:
  
  XVList() { head=NULL; count=0; };
  
  int  list_length() { return count; }

  void append(XVNode * new_node) { 
    
    XVNode * ptr = head; 
    count++;
    new_node->succ = NULL;
    if(!head){ head = new_node; return; }
    while( ptr->succ ) { ptr = (ptr->succ); }
    ptr->succ = new_node;
  };
  
  void remove(XVNode * rem_node) {
    
    XVNode *ptr=head;
    if(head == rem_node){
      head = head->succ; 
      count--; 
      return;
    }
    
    while(ptr && ptr->succ != rem_node) 
      ptr = ptr->succ;
    
    if(ptr){
      ptr->succ = (ptr->succ)->succ;
      count--;
    }
  };

  bool contains(XVNode * check) {
    
    XVNode * ptr = head;
    while( ptr ) { 
      if( ptr == check ) return true;
      ptr = ptr->succ;
    }
    return false;
  };

  bool empty() { return count > 0 ? false : true; };
};

#endif
