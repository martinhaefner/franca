#include <iostream>


#include "model.h"
#include "builder.h"


namespace fm = franca::model;


#if 0
bool depends(const fm::type& lhs, const fm::type& rhs)
{
   union 
   {
      fm::union_*      u;
      fm::struct_*     s;
      fm::enumeration* e;
      fm::typedef_*    t;      
      fm::array_*      a;
      fm::map_*        m;         
   } u;
   
   if ((u.u = dynamic_cast<fm::union_*>(&lhs)) != 0)      
   {
   }
   else if ((u.s = dynamic_cast<fm::struct_*>(&lhs)) != 0)      
   {
   }
   else if ((u.e = dynamic_cast<fm::enumeration*>(&lhs)) != 0)      
   {
   }       
   else if ((u.t = dynamic_cast<fm::typedef_*>(&lhs)) != 0)      
   {
   }       
   else if ((u.a = dynamic_cast<fm::array*>(&lhs)) != 0)      
   {
   }       
   else if ((u.m = dynamic_cast<fm::map*>(&lhs)) != 0)      
   {
   }       
}
#endif


void collectTypes(std::vector<const fm::type*>& typelist, const fm::typecollection& c)
{
   std::for_each(c.types_.begin(), c.types_.end(), [&typelist](const fm::type* t){
      typelist.push_back(t);
   });
}


void collectTypes(std::vector<const fm::type*>& typelist, const fm::package& p)
{
   // all packages
   std::for_each(p.packages_.begin(), p.packages_.end(), [&typelist](const fm::package& p){   
      collectTypes(typelist, p);
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


int main(int argc, const char** argv)
{
   if (argc < 2)
   {
      std::cerr << "no filename given" << std::endl;
      return EXIT_FAILURE;
   }
   
   fm::package root;
   
   try
   {      
      franca::builder::parse_and_build(root, argv[1]);
   }
   catch(std::exception& ex)
   {
      std::cerr << "Error: " << ex.what() << std::endl;
      return EXIT_FAILURE;
   }
   
   std::vector<const fm::type*> typelist;
   collectTypes(typelist, root);

   std::cout << "--- unsorted -------------------" << std::endl;
   
   std::for_each(typelist.begin(), typelist.end(), [](const fm::type* t){
      std::cout << t->fqn(".") << std::endl;
   });

   std::cout << "--- sorted ---------------------" << std::endl;   
   
   std::vector<const fm::type*> sorted_typelist;
   
   for (auto iter = typelist.begin(); iter != typelist.end(); ++iter)
   {      
      if (sorted_typelist.empty())
      {
         sorted_typelist.push_back(*iter);
      }
      else
      {
         auto target = sorted_typelist.begin();         
#if 0               
         std::cout << "------------" << std::endl;
#endif         
         
         while(target != sorted_typelist.end())               
         {                  
            bool r1 = (*iter)->depends(**target);
            bool r2 = (*target)->depends(**iter);
            
#if 0            
            if (r1)
            {
               std::cout << (*iter)->name() << " depends on " << (*target)->name() << std::endl;
            }
            else
               std::cout << (*iter)->name() << " does NOT depend on " << (*target)->name() << std::endl;
            
            if (r2)
            {
               std::cout << (*target)->name() << " depends on " << (*iter)->name() << std::endl;
            }
            else
               std::cout << (*target)->name() << " does NOT depend on " << (*iter)->name() << std::endl;
#endif
            
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
   
   std::for_each(sorted_typelist.begin(), sorted_typelist.end(), [](const fm::type* t){
      std::cout << t->fqn(".") << std::endl;
   });
   
   return EXIT_SUCCESS;
}
