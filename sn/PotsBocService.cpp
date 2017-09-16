//==============================================================================
//
//  PotsBocService.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "PotsBocService.h"
#include "BcCause.h"
#include "BcSessions.h"
#include "Context.h"
#include "Debug.h"
#include "EventHandler.h"
#include "PotsFeatures.h"
#include "PotsProfile.h"
#include "PotsSessions.h"
#include "SbAppIds.h"
#include "SbEvents.h"
#include "ServiceSM.h"
#include "Singleton.h"
#include "State.h"
#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace PotsBase
{
class PotsBocNull : public State
{
   friend class Singleton< PotsBocNull >;
private:
   PotsBocNull();
};

class PotsBocSsm : public ServiceSM
{
public:
   PotsBocSsm();
   ~PotsBocSsm();
private:
   virtual ServicePortId CalcPort(const AnalyzeMsgEvent& ame) override;
   virtual EventHandler::Rc ProcessInitAck
      (Event& currEvent, Event*& nextEvent) override;
   virtual EventHandler::Rc ProcessInitNack
      (Event& currEvent, Event*& nextEvent) override;
};

//==============================================================================

fn_name PotsBocInitiator_ctor = "PotsBocInitiator.ctor";

PotsBocInitiator::PotsBocInitiator() : Initiator(PotsBocServiceId,
   PotsCallServiceId, BcTrigger::AuthorizeOriginationSap,
   PotsAuthorizeOriginationSap::PotsBocPriority)
{
   Debug::ft(PotsBocInitiator_ctor);
}

//------------------------------------------------------------------------------

fn_name PotsBocInitiator_ProcessEvent = "PotsBocInitiator.ProcessEvent";

EventHandler::Rc PotsBocInitiator::ProcessEvent
   (const ServiceSM& parentSsm, Event& icEvent, Event*& ogEvent) const
{
   Debug::ft(PotsBocInitiator_ProcessEvent);

   auto& pssm = static_cast< const PotsBcSsm& >(parentSsm);
   auto prof = pssm.Profile();

   if(prof->HasFeature(BOC))
   {
      ogEvent = new InitiationReqEvent(*icEvent.Owner(), PotsBocServiceId);
      return EventHandler::Initiate;
   }

   return EventHandler::Pass;
}

//==============================================================================

fn_name PotsBocService_ctor = "PotsBocService.ctor";

PotsBocService::PotsBocService() : Service(PotsBocServiceId, false, true)
{
   Debug::ft(PotsBocService_ctor);

   Singleton< PotsBocNull >::Instance();
}

//------------------------------------------------------------------------------

fn_name PotsBocService_dtor = "PotsBocService.dtor";

PotsBocService::~PotsBocService()
{
   Debug::ft(PotsBocService_dtor);
}

//------------------------------------------------------------------------------

fn_name PotsBocService_AllocModifier = "PotsBocService.AllocModifier";

ServiceSM* PotsBocService::AllocModifier() const
{
   Debug::ft(PotsBocService_AllocModifier);

   return new PotsBocSsm;
}

//==============================================================================

fn_name PotsBocNull_ctor = "PotsBocNull.ctor";

PotsBocNull::PotsBocNull() : State(PotsBocServiceId, ServiceSM::Null)
{
   Debug::ft(PotsBocNull_ctor);
}

//==============================================================================

fn_name PotsBocSsm_ctor = "PotsBocSsm.ctor";

PotsBocSsm::PotsBocSsm() : ServiceSM(PotsBocServiceId)
{
   Debug::ft(PotsBocSsm_ctor);
}

//------------------------------------------------------------------------------

fn_name PotsBocSsm_dtor = "PotsBocSsm.dtor";

PotsBocSsm::~PotsBocSsm()
{
   Debug::ft(PotsBocSsm_dtor);
}

//------------------------------------------------------------------------------

fn_name PotsBocSsm_CalcPort = "PotsBocSsm.CalcPort";

ServicePortId PotsBocSsm::CalcPort(const AnalyzeMsgEvent& ame)
{
   Debug::ft(PotsBocSsm_CalcPort);

   return Parent()->CalcPort(ame);
}

//------------------------------------------------------------------------------

fn_name PotsBocSsm_ProcessInitAck = "PotsBocSsm.ProcessInitAck";

EventHandler::Rc PotsBocSsm::ProcessInitAck
   (Event& currEvent, Event*& nextEvent)
{
   Debug::ft(PotsBocSsm_ProcessInitAck);

   auto& pssm = *Parent();
   auto stid = pssm.CurrState();

   if(stid == BcState::AuthorizingOrigination)
   {
      pssm.SetNextSap(BcTrigger::OriginationDeniedSap);
      nextEvent = new BcOriginationDeniedEvent
         (pssm, Cause::OutgoingCallsBarred);
      return EventHandler::Revert;
   }

   Context::Kill(PotsBocSsm_ProcessInitAck, stid, 0);
   return EventHandler::Suspend;
}

//------------------------------------------------------------------------------

fn_name PotsBocSsm_ProcessInitNack = "PotsBocSsm.ProcessInitNack";

EventHandler::Rc PotsBocSsm::ProcessInitNack
   (Event& currEvent, Event*& nextEvent)
{
   Debug::ft(PotsBocSsm_ProcessInitNack);

   return EventHandler::Resume;
}
}