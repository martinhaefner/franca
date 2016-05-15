#include "model.h"
#include "builder.h"


namespace fm = franca::model;


namespace {

void collectTypes(std::vector<const fm::type*>& typelist, const fm::typecollection& c)
{
   std::for_each(c.types_.begin(), c.types_.end(), [&typelist](const fm::type* t){
      typelist.push_back(t);
   });
}


void collectTypes(std::vector<const fm::type*>& typelist, const fm::package& p)
{
   // all packages
   std::for_each(p.packages_.begin(), p.packages_.end(), [&typelist](const std::shared_ptr<fm::package>& p){   
      collectTypes(typelist, *p);
   });
   
   // typecollections...
   std::for_each(p.collections_.begin(), p.collections_.end(), [&typelist](const fm::typecollection& tc){
      collectTypes(typelist, tc);
   });
   
   // ...and interaces
   std::for_each(p.interfaces_.begin(), p.interfaces_.end(), [&typelist](const fm::interface& i){
      collectTypes(typelist, i);
   });
}


// ---------------------------------------------------------------------


void clearTypes(fm::typecollection& c)
{
   c.types_.clear();   
}


void clearTypes(fm::package& p)
{
   // all packages
   std::for_each(p.packages_.begin(), p.packages_.end(), [](std::shared_ptr<fm::package>& p){   
      clearTypes(*p);
   });
   
   // typecollections...
   std::for_each(p.collections_.begin(), p.collections_.end(), [](fm::typecollection& tc){
      clearTypes(tc);
   });
   
   // ...and interaces
   std::for_each(p.interfaces_.begin(), p.interfaces_.end(), [](fm::interface& i){
      clearTypes(i);
   });
}

}   // anonymous namespace


// ---------------------------------------------------------------------


/*static*/ 
void franca::builder::sort_types(fm::package& pkg)
{
   std::vector<const fm::type*> typelist;
   std::vector<const fm::type*> sorted_typelist;
   
   // collect all franca defined types in a single list for dependency sorting   
   collectTypes(typelist, pkg);
   
   for (auto iter = typelist.begin(); iter != typelist.end(); ++iter)
   {      
      if (sorted_typelist.empty())
      {
         sorted_typelist.push_back(*iter);
      }
      else
      {
         auto target = sorted_typelist.begin();         
         
         while(target != sorted_typelist.end())               
         {                  
            bool r1 = (*iter)->depends(**target);
            bool r2 = (*target)->depends(**iter);
            
            if (r1)
            {
               if (r2)
               {
                  assert(true);   // circular reference encountered
               }
               else
               {
                  // move on, try next
               }
            }
            else
            {
               if (r2)
               {                  
                  break;   // jump out, need to insert it here
               }
               else
               {
                  // completely independent
               }
            }
               
            ++target;            
         }      
         
         sorted_typelist.insert(target, *iter);
      }
   }
   
   // clear all type collections!
   clearTypes(pkg);
   
   // reinsert types into their type collections in correct order...   
   std::for_each(sorted_typelist.begin(), sorted_typelist.end(), [](const fm::type* t){
      t->parent_->types_.push_back(const_cast<fm::type*>(t));
   });   
}
