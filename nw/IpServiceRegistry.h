//==============================================================================
//
//  IpServiceRegistry.h
//
//  Copyright (C) 2017  Greg Utas
//
//  This file is part of the Robust Services Core (RSC).
//
//  RSC is free software: you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software
//  Foundation, either version 3 of the License, or (at your option) any later
//  version.
//
//  RSC is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
//  details.
//
//  You should have received a copy of the GNU General Public License along
//  with RSC.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef IPSERVICEREGISTRY_H_INCLUDED
#define IPSERVICEREGISTRY_H_INCLUDED

#include "Protected.h"
#include <string>
#include "NbTypes.h"
#include "Registry.h"

namespace NetworkBase
{
   class IpService;
}

//------------------------------------------------------------------------------

namespace NetworkBase
{
//  Global registry for protocols supported over IP.
//
class IpServiceRegistry : public NodeBase::Protected
{
   friend class IpService;
   friend class NodeBase::Singleton< IpServiceRegistry >;
public:
   //  Returns the service registered against NAME.
   //
   IpService* GetService(const std::string& name) const;

   //  Overridden for restarts.
   //
   void Startup(NodeBase::RestartLevel level) override;

   //  Overridden to display member variables.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden for patching.
   //
   void Patch(sel_t selector, void* arguments) override;
private:
   //  Private because this singleton is not subclassed.
   //
   IpServiceRegistry();

   //  Private because this singleton is not subclassed.
   //
   ~IpServiceRegistry();

   //  Adds SERVICE to the registry.  Invoked by IpService's base class
   //  constructor.
   //
   bool BindService(IpService& service);

   //  Removes SERVICE from the registry.  Invoked by IpService's base
   //  class destructor.
   //
   void UnbindService(IpService& service);

   //  The global registry of IP services.
   //
   NodeBase::Registry< IpService > services_;
};
}
#endif
