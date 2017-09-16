//==============================================================================
//
//  PotsShelfHandler.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "PotsShelf.h"
#include <memory>
#include <sstream>
#include "Circuit.h"
#include "Debug.h"
#include "LocalAddress.h"
#include "Log.h"
#include "Message.h"
#include "MsgHeader.h"
#include "PotsProtocol.h"
#include "PotsTrafficThread.h"
#include "SbAppIds.h"
#include "SbIpBuffer.h"
#include "Singleton.h"
#include "TlvParameter.h"

//------------------------------------------------------------------------------

namespace PotsBase
{
fn_name PotsShelfHandler_ctor = "PotsShelfHandler.ctor";

PotsShelfHandler::PotsShelfHandler(IpPort* port) :
   SbExtInputHandler(port)
{
   Debug::ft(PotsShelfHandler_ctor);
}

//------------------------------------------------------------------------------

fn_name PotsShelfHandler_dtor = "PotsShelfHandler.dtor";

PotsShelfHandler::~PotsShelfHandler()
{
   Debug::ft(PotsShelfHandler_dtor);
}

//------------------------------------------------------------------------------

fn_name PotsShelfHandler_ReceiveBuff = "PotsShelfHandler.ReceiveBuff";

void PotsShelfHandler::ReceiveBuff
   (MsgSize size, IpBufferPtr& buff, Faction faction) const
{
   Debug::ft(PotsShelfHandler_ReceiveBuff);

   auto sbuff = static_cast< SbIpBuffer* >(buff.get());
   auto header = sbuff->Header();
   auto pptr = reinterpret_cast< TlvParmPtr >(sbuff->PayloadPtr());
   auto phi = reinterpret_cast< PotsHeaderInfo* >(pptr->bytes);

   //  Verify that the message is addressed to an existing POTS circuit.
   //
   auto cct = Singleton< Switch >::Instance()->GetCircuit(phi->port);

   if(cct == nullptr) return;

   if(!cct->Supports(PotsProtocolId))
   {
      buff->InvalidDiscarded();

      auto log = Log::Create("POTS SHELF INVALID INCOMING BUFFER");

      if(log != nullptr)
      {
         *log << "port=" << phi->port;
         *log << "sig=" << phi->signal << CRLF;
         Log::Spool(log);
      }

      return;
   }

   header->route = Message::External;
   header->protocol = PotsProtocolId;
   header->signal = phi->signal;
   header->length = size;

   header->initial = false;
   header->final = (phi->signal != PotsSignal::Supervise);
   header->priority = Message::Progress;
   header->rxAddr.fid = PotsShelfFactoryId;
   header->txAddr.fid = PotsCallFactoryId;

   //  If traffic is running, give the shelf absolutely priority over the
   //  call server so that the call server will enter overload first.
   //
   auto thr = Singleton< PotsTrafficThread >::Extant();

   if((thr != nullptr) && (thr->GetRate() > 0))
   {
      header->priority = Message::Immediate;
   }

   //  Invoke the base class implementation to queue the message.  The base
   //  class assumes that SIZE includes a header.  The original message didn't
   //  have the header, but now it does, so adjust the length accordingly.
   //
   SbExtInputHandler::ReceiveBuff(sizeof(MsgHeader) + size, buff, faction);
}
}