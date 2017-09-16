//==============================================================================
//
//  SbTrace.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "SbTrace.h"
#include <iomanip>
#include <ios>
#include <sstream>
#include "Context.h"
#include "Debug.h"
#include "Factory.h"
#include "FactoryRegistry.h"
#include "Formatters.h"
#include "GlobalAddress.h"
#include "InvokerThread.h"
#include "MsgHeader.h"
#include "MsgPort.h"
#include "Protocol.h"
#include "ProtocolRegistry.h"
#include "ProtocolSM.h"
#include "RootServiceSM.h"
#include "SbEvents.h"
#include "SbIpBuffer.h"
#include "Service.h"
#include "ServiceRegistry.h"
#include "Signal.h"
#include "Singleton.h"
#include "State.h"
#include "Timer.h"
#include "ToolTypes.h"
#include "TraceBuffer.h"
#include "TraceDump.h"

using std::ostream;
using std::setw;
using std::string;

//------------------------------------------------------------------------------

namespace SessionBase
{
TransTrace::TransTrace(const Message& msg, const Factory& fac) :
   TimedRecord(sizeof(TransTrace), TransTracer),
   rcvr_(&fac),
   buff_(msg.Buffer()),
   ticks0_(0),
   ticks1_(0),
   cid_(fac.Fid()),
   service_(false),
   type_(fac.GetType()),
   prio_(msg.Header()->priority),
   prid_(msg.GetProtocol()),
   sid_(msg.GetSignal())
{
   ticks0_ = msg.Buffer()->RxTicks();
   ticks1_ = ticks0_;
   rid_ = RxNet;
}

//------------------------------------------------------------------------------

TransTrace::TransTrace
   (const Context& ctx, const Message& msg, const InvokerThread* inv) :
   TimedRecord(sizeof(TransTrace), TransTracer),
   rcvr_(&ctx),
   buff_(msg.Buffer()),
   ticks0_(0),
   ticks1_(0),
   cid_(NIL_ID),
   service_(false),
   type_(ctx.Type()),
   prio_(msg.Header()->priority),
   prid_(msg.GetProtocol()),
   sid_(msg.GetSignal())
{
   auto fac = msg.RxFactory();
   cid_ = fac->Fid();

   switch(type_)
   {
   case SingleMsg:
      rcvr_ = fac;
      break;

   case MultiPort:
      auto root = ctx.RootSsm();

      if(root != nullptr)
      {
         cid_ = root->Sid();
         service_ = true;
      }
   }

   ticks0_ = inv->Ticks0();
   ticks1_ = ticks0_;
   rid_ = Trans;
}

//------------------------------------------------------------------------------

bool TransTrace::Display(ostream& stream)
{
   if(!TimedRecord::Display(stream)) return false;

   auto delta = ticks1_ - ticks0_;
   auto usecs = Clock::TicksToUsecs(delta);
   stream << setw(TraceDump::TotWidth) << usecs << TraceDump::Tab();

   stream << rcvr_ << TraceDump::Tab();

   stream << type_ << SPACE;

   if(rid_ == Trans)
      stream << "prio=" << int(prio_);
   else
      stream << spaces(TraceDump::IdRcWidth - 4);

   stream << TraceDump::Tab();

   if(rid_ == RxNet)
   {
      if(service_)
      {
         auto reg = Singleton< ServiceRegistry >::Instance();
         stream << strClass(reg->GetService(ServiceId(cid_)), false);
      }
      else
      {
         auto reg = Singleton< FactoryRegistry >::Instance();
         stream << strClass(reg->GetFactory(FactoryId(cid_)), false);
      }
   }
   else
   {
      auto reg = Singleton< ProtocolRegistry >::Instance();
      auto pro = reg->GetProtocol(prid_);

      if(pro != nullptr) stream << strClass(pro->GetSignal(sid_), false);
   }

   return true;
}

//------------------------------------------------------------------------------

void TransTrace::EndOfTransaction()
{
   //  Set the time at which this transaction ended.
   //
   ticks1_ = Clock::TicksNow();
}

//------------------------------------------------------------------------------

fixed_string RxNetEventStr = "RXNET";
fixed_string TransEventStr = "TRANS";

const char* TransTrace::EventString() const
{
   switch(rid_)
   {
   case RxNet: return RxNetEventStr;
   case Trans: return TransEventStr;
   }

   return ERROR_STR;
}

//------------------------------------------------------------------------------

void TransTrace::ResumeTime(const ticks_t& then)
{
   //  Adjust this transaction's elapsed time so that the time spent since
   //  THEN is excluded.
   //
   auto warp = Clock::TicksSince(then);
   ticks0_ += warp;
   ticks1_ = ticks0_;
}

//------------------------------------------------------------------------------

void TransTrace::SetContext(const void* ctx)
{
   //  This should only be invoked on an RxNet record once the context is
   //  known.  If the context is a MsgFactory, retain the factory as the
   //  receiver.
   //
   if((rid_ == RxNet) && (type_ != SingleMsg))
   {
      rcvr_ = ctx;
   }
}

//------------------------------------------------------------------------------

void TransTrace::SetService(ServiceId sid)
{
   cid_ = sid;
   service_ = true;
}

//==============================================================================

BuffTrace::BuffTrace(Id rid, const SbIpBuffer& buff) :
   TimedRecord(sizeof(BuffTrace), BufferTracer),
   buff_(nullptr),
   verified_(false),
   corrupt_(false)
{
   buff_ = new (ToolUser) SbIpBuffer(buff);
   rid_ = rid;
}

//------------------------------------------------------------------------------

BuffTrace::~BuffTrace()
{
   //e If a StTestData::lastMsg_ or TestSession::lastMsg_ points to this record,
   //  it will probably lead to a trap.  The odds of this are remote because it
   //  means that the trace buffer wrapped around and caught up with the last
   //  message verified by the factory or PSM.

   //  If our SbIpBuffer is corrupt, we will trap, and we must not trap again
   //  during cleanup.  Flag the buffer as corrupt before deleting it and
   //  clear the flag afterwards.  If it flagged as corrupt when we come in
   //  here, we know that it was bad, so skip it and let the audit find it.
   //
   if((buff_ != nullptr) && !buff_->IsInvalid())
   {
      if(!corrupt_)
      {
         corrupt_ = true;
         delete buff_;
      }

      buff_ = nullptr;
      corrupt_ = false;
   }
}

//------------------------------------------------------------------------------

FactoryId BuffTrace::ActiveFid() const
{
   auto header = Header();
   if(header == nullptr) return NIL_ID;
   if(rid_ == IcMsg) return header->rxAddr.fid;
   return header->txAddr.fid;
}

//------------------------------------------------------------------------------

fn_name BuffTrace_ClaimBlocks = "BuffTrace.ClaimBlocks";

void BuffTrace::ClaimBlocks()
{
   Debug::ft(BuffTrace_ClaimBlocks);

   if((buff_ != nullptr) && !buff_->IsInvalid() && !corrupt_)
   {
      buff_->Claim();
   }
}

//------------------------------------------------------------------------------

bool BuffTrace::Display(ostream& stream)
{
   if(!TimedRecord::Display(stream)) return false;

   stream << CRLF;
   stream << string(80, '-') << CRLF;

   if(buff_ == nullptr)
   {
      stream << "No buffer found." << CRLF;
      stream << string(80, '-');
      return true;
   }

   auto fid = ActiveFid();
   auto fac = Singleton< FactoryRegistry >::Instance()->GetFactory(fid);
   stream << "factory=" << int(fid);
   stream << " (" << strClass(fac, false) << ')' << CRLF;

   if(!buff_->IsInvalid()) buff_->Display(stream, spaces(2), Flags(Vb_Mask));
   stream << string(80, '-');
   return true;
}

//------------------------------------------------------------------------------

fixed_string IcMsgEventStr = "icmsg";
fixed_string OgMsgEventStr = "ogmsg";

const char* BuffTrace::EventString() const
{
   switch(rid_)
   {
   case IcMsg: return IcMsgEventStr;
   case OgMsg: return OgMsgEventStr;
   }

   return ERROR_STR;
}

//------------------------------------------------------------------------------

MsgHeader* BuffTrace::Header() const
{
   if(buff_ == nullptr) return nullptr;
   return buff_->Header();
}

//------------------------------------------------------------------------------

fn_name BuffTrace_NextIcMsg = "BuffTrace.NextIcMsg";

BuffTrace* BuffTrace::NextIcMsg
   (BuffTrace* bt, FactoryId fid, SignalId sid, SkipInfo& skip)
{
   Debug::ft(BuffTrace_NextIcMsg);

   auto buff = Singleton< TraceBuffer >::Instance();

   buff->Lock();
   {
      TraceRecord* rec = bt;
      auto max = 200;
      auto mask = Flags(1 << BufferTracer);

      for(buff->Next(rec, mask); rec != nullptr; buff->Next(rec, mask))
      {
         //  Skip messages that were already verified or that were injected.
         //
         bt = static_cast< BuffTrace* >(rec);
         if(bt->verified_) continue;

         auto header = bt->Header();
         if(header->injected) continue;

         //  When a message bypasses the IP stack, the trace only captures
         //  the outgoing message, so that is what we must use.  But when a
         //  message arrives over the IP stack, we need to use the incoming
         //  message because
         //  o if the message was interprocessor, only another processor
         //    could have captured the outgoing message;
         //  o if the message was an *initial* intraprocessor message, only
         //    the incoming message contains the recipient's address (which
         //    is not known until a MsgPort is allocated), and we need that
         //    address in order to find the test SSM and PSM.
         //
         auto rid = rec->Rid();
         if((rid == OgMsg) && (header->route != Message::Internal)) continue;

         //  Skip messages that do not match FID and SID, but note the first
         //  one's signal and count them.
         //
         if(header->rxAddr.fid == fid)
         {
            if(header->signal == sid)
            {
               buff->Unlock();
               return bt;
            }

            if(header->signal != Signal::Timeout)
            {
               if(skip.count == 0)
               {
                  skip.first = header->signal;
               }

               ++skip.count;
            }
         }

         if(--max <= 0)
         {
            Debug::SwErr(BuffTrace_NextIcMsg, 200, 0);
            break;
         }
      }
   }
   buff->Unlock();
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name BuffTrace_Rewrap = "BuffTrace.Rewrap";

Message* BuffTrace::Rewrap()
{
   Debug::ft(BuffTrace_Rewrap);

   if(buff_ == nullptr) return nullptr;

   auto reg = Singleton< FactoryRegistry >::Instance();
   auto fac = reg->GetFactory(Header()->rxAddr.fid);
   auto ipb = SbIpBufferPtr(new (ToolUser) SbIpBuffer(*buff_));

   verified_ = true;
   if(ipb == nullptr) return nullptr;
   return fac->ReallocOgMsg(ipb);
}

//------------------------------------------------------------------------------

fn_name BuffTrace_Shutdown = "BuffTrace.Shutdown";

void BuffTrace::Shutdown(RestartLevel level)
{
   Debug::ft(BuffTrace_Shutdown);

   if(level >= RestartCold) Nullify();
}

//==============================================================================

SboTrace::SboTrace(size_t size, const Pooled& sbo) :
   TimedRecord(size, ContextTracer),
   sbo_(&sbo)
{
}

//------------------------------------------------------------------------------

bool SboTrace::Display(ostream& stream)
{
   if(!TimedRecord::Display(stream)) return false;

   stream << spaces(TraceDump::EvtToObj) << sbo_ << TraceDump::Tab();
   return true;
}

//------------------------------------------------------------------------------

string SboTrace::OutputId(const string& label, id_t id)
{
   auto width = TraceDump::IdRcWidth + TraceDump::TabWidth;
   if(id == NIL_ID) return spaces(width);
   width -= col_t(label.size());

   std::ostringstream stream;
   stream << label;
   stream << std::left << std::setw(width) << std::setfill(SPACE) << id;
   return stream.str();
}

//==============================================================================

SsmTrace::SsmTrace(Id rid, const ServiceSM& ssm) :
   SboTrace(sizeof(SsmTrace), ssm),
   sid_(ssm.Sid())
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool SsmTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   auto reg = Singleton< ServiceRegistry >::Instance();

   stream << spaces(TraceDump::ObjToDesc);
   stream << strClass(reg->GetService(sid_), false);

   return true;
}

//------------------------------------------------------------------------------

fixed_string SsmCreationEventStr = " +ssm";
fixed_string SsmDeletionEventStr = " -ssm";

const char* SsmTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return SsmCreationEventStr;
   case Deletion: return SsmDeletionEventStr;
   }

   return SboTrace::EventString();
}

//==============================================================================

PsmTrace::PsmTrace(Id rid, const ProtocolSM& psm) :
   SboTrace(sizeof(PsmTrace), psm),
   fid_(psm.GetFactory()),
   bid_(NIL_ID)
{
   auto port = psm.Port();
   if(port != nullptr) bid_ = port->LocAddr().SbAddr().bid;
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool PsmTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   auto reg = Singleton< FactoryRegistry >::Instance();

   stream << OutputId("port=", bid_);
   stream << strClass(reg->GetFactory(fid_), false);

   return true;
}

//------------------------------------------------------------------------------

fixed_string PsmCreationEventStr = " +psm";
fixed_string PsmDeletionEventStr = " -psm";

const char* PsmTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return PsmCreationEventStr;
   case Deletion: return PsmDeletionEventStr;
   }

   return SboTrace::EventString();
}

//==============================================================================

PortTrace::PortTrace(Id rid, const MsgPort& port) :
   SboTrace(sizeof(PortTrace), port),
   fid_(port.ObjAddr().fid),
   bid_(port.ObjAddr().bid)
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool PortTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   auto reg = Singleton< FactoryRegistry >::Instance();

   stream << OutputId("port=", bid_);
   stream << strClass(reg->GetFactory(fid_), false);

   return true;
}

//------------------------------------------------------------------------------

fixed_string PortCreationEventStr = "+port";
fixed_string PortDeletionEventStr = "-port";

const char* PortTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return PortCreationEventStr;
   case Deletion: return PortDeletionEventStr;
   }

   return SboTrace::EventString();
}

//==============================================================================

MsgTrace::MsgTrace(Id rid, const Message& msg, Message::Route route) :
   SboTrace(sizeof(MsgTrace), msg),
   prid_(msg.GetProtocol()),
   sid_(msg.GetSignal()),
   locAddr_(NilLocalAddress),
   remAddr_(NilLocalAddress),
   route_(route),
   noCtx_(Context::RunningContext() == nullptr),
   self_(msg.Header()->self)
{
   ProtocolSM* psm;

   switch(rid)
   {
   case Creation:
   case Deletion:
      psm = msg.Psm();
      if(psm != nullptr)
      {
         auto port = psm->Port();
         if(port != nullptr) locAddr_.bid = port->ObjAddr().bid;
      }
      break;

   case Reception:
   case Transmission:
      if((rid == Reception) || self_)
      {
         locAddr_ = msg.RxSbAddr();
         remAddr_ = msg.TxSbAddr();
      }
      else
      {
         locAddr_ = msg.TxSbAddr();
         remAddr_ = msg.RxSbAddr();
      }
   }

   rid_ = rid;
}

//------------------------------------------------------------------------------

bool MsgTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << OutputId("port=", locAddr_.bid);

   auto pro = Singleton< ProtocolRegistry >::Instance()->GetProtocol(prid_);
   Signal* sig = nullptr;

   if(pro != nullptr) sig = pro->GetSignal(sid_);

   if(sig != nullptr)
   {
      stream << strClass(sig, false);
   }
   else
   {
      if(pro != nullptr)
         stream << strClass(pro, false);
      else
         stream << "pro=" << prid_;

      stream << " sig=" << sid_;
   }

   return true;
}

//------------------------------------------------------------------------------

fixed_string MsgCreationEventStr     = " +msg";
fixed_string MsgDeletionEventStr     = " -msg";
fixed_string MsgReceptionEventStr    = ">>msg";
fixed_string MsgTransmissionEventStr = "<<msg";

const char* MsgTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return MsgCreationEventStr;
   case Deletion: return MsgDeletionEventStr;
   case Reception: return MsgReceptionEventStr;
   case Transmission: return MsgTransmissionEventStr;
   }

   return SboTrace::EventString();
}

//==============================================================================

TimerTrace::TimerTrace(Id rid, const Timer& tmr) :
   SboTrace(sizeof(TimerTrace), tmr),
   tid_(tmr.Tid()),
   secs_(tmr.duration_),
   psm_(tmr.Psm())
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool TimerTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << OutputId("id=", tid_);
   stream << "secs=" << secs_ << " psm=" << psm_;
   return true;
}

//------------------------------------------------------------------------------

fixed_string TimerCreationEventStr = " +tmr";
fixed_string TimerDeletionEventStr = " -tmr";

const char* TimerTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return TimerCreationEventStr;
   case Deletion: return TimerDeletionEventStr;
   }

   return SboTrace::EventString();
}

//==============================================================================

EventTrace::EventTrace(Id rid, const Event& evt) :
   SboTrace(sizeof(EventTrace), evt),
   owner_(NIL_ID),
   eid_(evt.Eid())
{
   auto ssm = evt.Owner();
   if(ssm != nullptr) owner_ = ssm->Sid();
   rid_ = rid;
}

//------------------------------------------------------------------------------

EventTrace::EventTrace(size_t size, const Event& evt) :
   SboTrace(size, evt),
   owner_(NIL_ID),
   eid_(evt.Eid())
{
   auto ssm = evt.Owner();
   if(ssm != nullptr) owner_ = ssm->Sid();
}

//------------------------------------------------------------------------------

bool EventTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << spaces(TraceDump::ObjToDesc);
   DisplayEvent(stream, owner_, eid_);
   return true;
}

//------------------------------------------------------------------------------

void EventTrace::DisplayEvent(ostream& stream, ServiceId sid, EventId eid)
{
   auto svc = Singleton< ServiceRegistry >::Instance()->GetService(sid);

   if(svc != nullptr)
   {
      auto name = svc->EventName(eid);

      if(name != nullptr)
      {
         stream << name;
         return;
      }

      stream << strClass(svc, true) << ", ";
   }
   else if(sid != NIL_ID)
   {
      stream << "svc=" << sid << ", ";
   }

   stream << "evt=" << eid;
}

//------------------------------------------------------------------------------

fixed_string EventCreationEventStr = " +evt";
fixed_string EventDeletionEventStr = " -evt";
fixed_string EventHandlerEventStr  = ">>evt";

const char* EventTrace::EventString() const
{
   switch(rid_)
   {
   case Creation: return EventCreationEventStr;
   case Deletion: return EventDeletionEventStr;
   }

   return EventHandlerEventStr;
}

//==============================================================================

HandlerTrace::HandlerTrace
   (ServiceId sid, const State& state, const Event& evt, EventHandler::Rc rc) :
   EventTrace(sizeof(HandlerTrace), evt),
   sid_(sid),
   stid_(state.Stid()),
   rc_(rc)
{
   rid_ = Handler;
}

//------------------------------------------------------------------------------

HandlerTrace::HandlerTrace(size_t size, ServiceId sid,
   const State& state, const Event& evt, EventHandler::Rc rc) :
   EventTrace(size, evt),
   sid_(sid),
   stid_(state.Stid()),
   rc_(rc)
{
}

//------------------------------------------------------------------------------

bool HandlerTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << rc_ << spaces(4);
   DisplayEvent(stream, sid_, eid_);
   stream << " >> ";
   DisplayState(stream);
   return true;
}

//------------------------------------------------------------------------------

void HandlerTrace::DisplayState(ostream& stream) const
{
   auto svc = Singleton< ServiceRegistry >::Instance()->GetService(sid_);

   if(svc != nullptr)
      stream << strClass(svc->GetState(stid_), false);
   else
      stream << "state=" << stid_;
}

//==============================================================================

SxpTrace::SxpTrace
   (ServiceId sid, const State& state, const Event& sxp, EventHandler::Rc rc) :
   HandlerTrace(sizeof(SxpTrace), sid, state, sxp, rc),
   curr_(0)
{
   switch(sxp.Eid())
   {
   case Event::AnalyzeSap:
      curr_ = static_cast< const AnalyzeSapEvent& >(sxp).CurrEvent()->Eid();
   case Event::AnalyzeSnp:
      curr_ = static_cast< const AnalyzeSnpEvent& >(sxp).CurrEvent()->Eid();
   }

   rid_ = SxpEvent;
}

//------------------------------------------------------------------------------

bool SxpTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << rc_ << spaces(4);
   DisplayEvent(stream, sid_, eid_);

   stream << '(';
   DisplayEvent(stream, owner_, curr_);
   stream << ')';

   stream << " >> ";
   DisplayState(stream);
   return true;
}

//==============================================================================

SipTrace::SipTrace
   (ServiceId sid, const State& state, const Event& sip, EventHandler::Rc rc) :
   HandlerTrace(sizeof(SipTrace), sid, state, sip, rc),
   mod_((static_cast< const InitiationReqEvent& >(sip)).GetModifier())
{
   rid_ = SipEvent;
}

//------------------------------------------------------------------------------

bool SipTrace::Display(ostream& stream)
{
   if(!SboTrace::Display(stream)) return false;

   stream << rc_ << spaces(4);
   DisplayEvent(stream, sid_, eid_);

   stream << '(';
   auto svc = Singleton< ServiceRegistry >::Instance()->GetService(mod_);
   if(svc != nullptr)
      stream << strClass(svc, false);
   else
      stream << "mod=" << mod_;
   stream << ')';

   stream << " >> ";
   DisplayState(stream);
   return true;
}
}