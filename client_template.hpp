@require(interface)


#ifndef @{interface.fqn('_').upper()}_HPP
#define @{interface.fqn('_').upper()}_HPP


namespace X
{
   
namespace Y
{

class @{interface.name}Client : public Client
{
public:
   @{interface.name}Client()
   {
      // NOOP
   }
   
   ~@{interface.name}Client()
   {
      // NOOP
   }

   @for m in interface.methods :
   void @{m.name}(@{m.in_args!!list_args}@{len(m.in_args) and ", " or ""}@{m.out_args!!list_args})
   {
      // TODO do anything
   }
   
   @end
   
   @for b in interface.broadcasts :
   virtual void @{b.name}(@{b.args!!list_args}) = 0;   
   
   @end
   
   @for m in interface.fire_and_forget_methods :
   void @{m.name}(@{m.args!!list_args})
   {
      send_request(...);
   }
   
   @end
   
private:
   
   @for a in interface.attributes :
   attribute<@{a.type().name}> @{a.name}_;   
   @end
};

}   // namespace

}   // namespace


#endif   // @{interface.fqn('_').upper()}_HPP
