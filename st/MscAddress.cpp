//==============================================================================
//
//  MscAddress.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "MscAddress.h"
#include <ostream>
#include <string>
#include "Algorithms.h"
#include "Debug.h"
#include "Message.h"
#include "SbTrace.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace SessionTools
{
fn_name MscAddress_ctor = "MscAddress.ctor";

MscAddress::MscAddress(const MsgTrace& mt, MscContext* context) :
   locAddr_(mt.LocAddr()),
   remAddr_(NilLocalAddress),
   context_(context),
   external_(false),
   extFid_(NIL_ID)
{
   Debug::ft(MscAddress_ctor);

   SetPeer(mt, context);
}

//------------------------------------------------------------------------------

fn_name MscAddress_dtor = "MscAddress.dtor";

MscAddress::~MscAddress()
{
   Debug::ft(MscAddress_dtor);
}

//------------------------------------------------------------------------------

void MscAddress::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Temporary::Display(stream, prefix, options);

   stream << prefix << "locAddr  : " << locAddr_.to_str() << CRLF;
   stream << prefix << "remAddr  : " << remAddr_.to_str() << CRLF;
   stream << prefix << "context  : " << context_ << CRLF;
   stream << prefix << "external : " << external_ << CRLF;
   stream << prefix << "extFid   : " << extFid_ << CRLF;
}

//------------------------------------------------------------------------------

bool MscAddress::ExternalFid(FactoryId& fid) const
{
   if(!external_) return false;
   fid = extFid_;
   return true;
}

//------------------------------------------------------------------------------

ptrdiff_t MscAddress::LinkDiff()
{
   int local;
   auto fake = reinterpret_cast< const MscAddress* >(&local);
   return ptrdiff(&fake->link_, fake);
}

//------------------------------------------------------------------------------

fn_name MscAddress_SetPeer = "MscAddress.SetPeer";

void MscAddress::SetPeer(const MsgTrace& mt, MscContext* context)
{
   Debug::ft(MscAddress_SetPeer);

   if(context_ == nullptr) context_ = context;

   if(mt.Route() == Message::Internal)
   {
      if(locAddr_.bid == mt.LocAddr().bid)
      {
         if(remAddr_.bid == NIL_ID)
         {
            remAddr_ = mt.RemAddr();
         }

         return;
      }

      if(locAddr_.bid == mt.RemAddr().bid)
      {
         if(remAddr_.bid == NIL_ID)
         {
            remAddr_ = mt.LocAddr();
         }

         return;
      }
   }
   else if(!external_)
   {
      external_ = true;
      extFid_ = mt.RemAddr().fid;
   }
   else if(extFid_ != mt.RemAddr().fid)
   {
      Debug::SwErr(MscAddress_SetPeer,
         pack2(extFid_, mt.RemAddr().fid), locAddr_.fid);
   }
}
}