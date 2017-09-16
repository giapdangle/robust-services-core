//==============================================================================
//
//  ServiceCodeRegistry.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "ServiceCodeRegistry.h"
#include <ostream>
#include <string>
#include "Debug.h"
#include "Formatters.h"
#include "SbAppIds.h"
#include "Service.h"
#include "ServiceRegistry.h"
#include "Singleton.h"
#include "SymbolRegistry.h"
#include "SysTypes.h"

using std::ostream;
using std::string;
using namespace SessionBase;

//------------------------------------------------------------------------------

namespace CallBase
{
fn_name ServiceCodeRegistry_ctor = "ServiceCodeRegistry.ctor";

ServiceCodeRegistry::ServiceCodeRegistry()
{
   Debug::ft(ServiceCodeRegistry_ctor);

   for(auto i = 0; i <= Address::LastSC; ++i)
   {
      codeToService_[i] = NIL_ID;
   }
}

//------------------------------------------------------------------------------

fn_name ServiceCodeRegistry_dtor = "ServiceCodeRegistry.dtor";

ServiceCodeRegistry::~ServiceCodeRegistry()
{
   Debug::ft(ServiceCodeRegistry_dtor);
}

//------------------------------------------------------------------------------

void ServiceCodeRegistry::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Protected::Display(stream, prefix, options);

   stream << prefix << "codeToService [Address::SC]" << CRLF;

   for(auto i = 0; i <= Address::LastSC; ++i)
   {
      auto lead = prefix + spaces(2);
      auto sid = codeToService_[i];

      if(sid != NIL_ID)
      {
         auto svc = Singleton< ServiceRegistry >::Instance()->GetService(sid);

         stream << lead << strIndex(i);

         if(svc != nullptr)
            stream << strObj(svc) << CRLF;
         else
            stream << sid << " (no service registered)" << CRLF;
      }
   }
}

//------------------------------------------------------------------------------

fn_name ServiceCodeRegistry_GetService = "ServiceCodeRegistry.GetService";

ServiceId ServiceCodeRegistry::GetService(Address::SC sc) const
{
   Debug::ft(ServiceCodeRegistry_GetService);

   if(Address::IsValidSC(sc)) return codeToService_[sc];

   Debug::SwErr(ServiceCodeRegistry_GetService, sc, 0);
   return NIL_ID;
}

//------------------------------------------------------------------------------

fn_name ServiceCodeRegistry_SetService = "ServiceCodeRegistry.SetService";

void ServiceCodeRegistry::SetService(Address::SC sc, ServiceId sid)
{
   Debug::ft(ServiceCodeRegistry_SetService);

   if(!Address::IsValidSC(sc))
   {
      Debug::SwErr(ServiceCodeRegistry_SetService, sc, 0);
      return;
   }

   codeToService_[sc] = sid;
}

//------------------------------------------------------------------------------

fn_name ServiceCodeRegistry_Startup = "ServiceCodeRegistry.Startup";

void ServiceCodeRegistry::Startup(RestartLevel level)
{
   Debug::ft(ServiceCodeRegistry_Startup);

   //  Define service codes and corresponding symbols.  These are
   //  fixed but would be configurable in a production system.
   //
   SetService(33, PotsWmlActivation);
   SetService(34, PotsWmlDeactivation);
   SetService(70, PotsCcwServiceId);
   SetService(72, PotsCfuActivation);
   SetService(73, PotsCfuDeactivation);

   if(level < RestartCold) return;

   auto reg = Singleton< SymbolRegistry >::Instance();
   reg->BindSymbol("sc.wml.activation", "*33");
   reg->BindSymbol("sc.wml.deactivation", "*34");
   reg->BindSymbol("sc.ccw", "*70");
   reg->BindSymbol("sc.cfu.activation", "*72");
   reg->BindSymbol("sc.cfu.deactivation", "*73");
}
}