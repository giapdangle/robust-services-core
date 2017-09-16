//==============================================================================
//
//  Service.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include "Protected.h"
#include <cstddef>
#include "Event.h"
#include "EventHandler.h"
#include "RegCell.h"
#include "Registry.h"
#include "SbTypes.h"
#include "State.h"
#include "SysTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace SessionBase
{
//  Each SessionBase application provides a singleton subclass, which contains
//  other singletons that define the application's state machine.
//
class Service : public Protected
{
   friend class Registry< Service >;
   friend class State;
public:
   //  Allows "Id" to refer to a service identifier in this class hierarchy.
   //
   typedef ServiceId Id;

   //> Highest valid service identifier.
   //
   static const Id MaxId = 511;

   //  Returns the service's identifier.
   //
   Id Sid() const { return Id(sid_.GetId()); }

   //  Allows "PortId" to refer to a port identifier in this class hierarchy.
   //
   typedef ServicePortId PortId;

   //  Port identifiers.
   //
   static const PortId UserPort = 1;     // direction towards user (client)
   static const PortId NetworkPort = 2;  // direction towards network (server)
   static const PortId NextPortId = 3;   // next available port identifier

   //  Returns true if PID is a valid port identifier.
   //
   static bool IsValidPortId(PortId pid)
   {
      return ((pid != NIL_ID) && (pid <= MaxServicePortId));
   }

   //  Returns a symbolic name for the port identified by PID.  Should be
   //  overridden by a subclass that defines an application-specific port.
   //
   virtual const char* PortName(PortId pid) const;

   //  Returns the state registered against SID.
   //
   State* GetState(State::Id stid) const
   {
      return states_.At(stid);
   }

   //  Returns the event handler registered against EHID.
   //
   EventHandler* GetHandler(EventHandlerId ehid) const
   {
      return handlers_.At(ehid);
   }

   //  Returns the name of the event associated with EID.
   //
   const char* EventName(EventId eid) const;

   //  Returns the trigger registered against TID.
   //
   Trigger* GetTrigger(TriggerId tid) const;

   //  Invoked by SessionBase to create a modifier's service state machine
   //  (SSM) during service initiation.  Must be overridden by a modifier
   //  to create an instance of its SSM:
   //
   //    ServiceSM* MySSM::AllocModifier() { return new MySSM; }
   //
   virtual ServiceSM* AllocModifier() const;

   //  Returns true if the service supports modifiers.
   //
   bool IsModifiable() const { return modifiable_; }

   //  Returns true if the service is a modifier.
   //
   bool IsModifier() const { return modifier_; }

   //  Takes a modifier out of service.  Instances of the modifier's state
   //  machine continue to run, but no new instances are created.  Calls to
   //  the modifier's initiators are suppressed.
   //
   bool Disable();

   //  Puts a modifier back into service.
   //
   bool Enable();

   //  The status of a service.
   //
   enum Status
   {
      NotRegistered,  // not in ServiceRegistry
      Disabled,       // creation of state machines disabled
      Enabled         // creation of state machines enabled
   };

   //  Returns a service's status.
   //
   Status GetStatus() const { return status_; }

   //  Returns the registry of states.  Used for iteration.
   //
   const Registry< State >& States() const { return states_; }

   //  Returns the registry of event handlers.  Used for iteration.
   //
   const Registry< EventHandler >& Handlers() const { return handlers_; }

   //  Returns the registry of triggers.  Used for iteration.
   //
   const Registry< Trigger >& Triggers() const { return triggers_; }

   //  Returns the offset to sid_.
   //
   static ptrdiff_t CellDiff();

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;
protected:
   //  Sets the corresponding member variables.  Sets the service's status to
   //  enabled, initializes its registries, registers system event handlers
   //  with it, and adds it to ServiceRegistry.  Protected because this class
   //  is virtual.
   //
   explicit Service(Id sid, bool modifiable = false, bool modifier = false);

   //  Deletes all states before removing the service from ServiceRegistry.
   //  Protected because subclasses should be singletons.
   //
   virtual ~Service();

   //  Registers HANDLER in the slot specified by EHID.  An event handler
   //  can be registered with more than one service, so individual service
   //  constructors must invoke this.
   //
   bool BindHandler(EventHandler& handler, EventHandlerId ehid);

   //  Registers NAME as the class name associated with EID.  A service
   //  should register a name for each event that it defines and invoke
   //  this function in its constructor.
   //
   bool BindEventName(const char* name, EventId eid);

   //  Registers TRIGGER with the service.  The service must be modifiable.
   //  A trigger can register with more than one service, so individual
   //  service constructors must invoke this.
   //
   bool BindTrigger(Trigger& trigger);
private:
   //  Adds STATE to the service.
   //
   bool BindState(State& state);

   //  Removes STATE from the service.
   //
   void UnbindState(State& state);

   //  Registers HANDLER against EHID.
   //
   bool BindSystemHandler(EventHandler& handler, EventHandlerId ehid);

   //  Overridden to prohibit copying.
   //
   Service(const Service& that);
   void operator=(const Service& that);

   //  The service's identifier.
   //
   RegCell sid_;

   //  Whether the service is registered and, if so, whether it is enabled.
   //
   Status status_;

   //  Registry for the service's states.
   //
   Registry< State > states_;

   //  Registry for the service's event handlers.
   //
   Registry< EventHandler > handlers_;

   //  Registry for the service's event names.
   //
   const char* eventNames_[Event::MaxId + 1];

   //  Registry for the service's triggers (if it is modifiable).
   //
   Registry< Trigger > triggers_;

   //  Set if the service suppports modifiers.
   //
   const bool modifiable_;

   //  Set if the service is a modifier.
   //
   const bool modifier_;
};
}
#endif