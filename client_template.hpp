@require(interface)


#ifndef @{interface.fqn('_').upper()}_HPP
#define @{interface.fqn('_').upper()}_HPP


@{interface.dependent_includes()}


@{interface.package().namespaces_open()}

class @{interface.name()}Client : public Client
{
public:
   @{interface.name()}Client()
   {
      // NOOP
   }
   
   ~@{interface.name()}Client()
   {
      // NOOP
   }

   @for m in interface.methods :
   void @{m.name()}(@{m.in_args.for_method_decl_in()}@{ (len(m.in_args) > 0 and len(m.out_args) > 0) and ', ' or '' }@{m.out_args.for_method_decl_out()})
   {
      // TODO do anything
   }
   
   @end
   
   @for b in interface.broadcasts :
   virtual void @{b.name()}(@{b.args.for_method_decl_in()}) = 0;   
   
   @end
   
   @for m in interface.fire_and_forget_methods :
   void @{m.name()}(@{m.args.for_method_decl_in()})
   {
      send_request(...);
   }
   
   @end
   
private:
   
   @for a in interface.attributes :
   attribute<@{a.type().cpp_type()}> @{a.name()}_;   
   @end
};


@{interface.package().namespaces_close()}

#endif   // @{interface.fqn('_').upper()}_HPP
