//==============================================================================
//
//  StatisticsThread.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "StatisticsThread.h"
#include <sstream>
#include <string>
#include "Debug.h"
#include "FileThread.h"
#include "Log.h"
#include "Singleton.h"
#include "StatisticsRegistry.h"
#include "SysTime.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NodeBase
{
secs_t StatisticsThread::LongIntervalSecs = 900;  // must be a multiple of 60
secs_t StatisticsThread::ShortIntervalSecs = 5;   // must be a divisor of 60

size_t StatisticsThread::WakeupsBetweenReports =
   StatisticsThread::LongIntervalSecs / StatisticsThread::ShortIntervalSecs;

ticks_t StatisticsThread::PrevToCurrTicks =
   Clock::SecsToTicks(StatisticsThread::ShortIntervalSecs);

//------------------------------------------------------------------------------

fn_name StatisticsThread_ctor = "StatisticsThread.ctor";

StatisticsThread::StatisticsThread() : Thread(BackgroundFaction),
   wakeupTicks_(0),
   countdown_(WakeupsBetweenReports),
   delayed_(false)
{
   Debug::ft(StatisticsThread_ctor);
}

//------------------------------------------------------------------------------

fn_name StatisticsThread_dtor = "StatisticsThread.dtor";

StatisticsThread::~StatisticsThread()
{
   Debug::ft(StatisticsThread_dtor);
}

//------------------------------------------------------------------------------

const char* StatisticsThread::AbbrName() const
{
   return "stats";
}

//------------------------------------------------------------------------------

fn_name StatisticsThread_CalcFirstDelay = "StatisticsThread.CalcFirstDelay";

msecs_t StatisticsThread::CalcFirstDelay()
{
   Debug::ft(StatisticsThread_CalcFirstDelay);

   auto timeNow = SysTime();
   auto ticksNow = Clock::TicksNow();

   //  Start the first short interval for thread statistics at the next
   //  time that is at least half the distance between short intervals.
   //
   auto tshort = timeNow;
   tshort.AddMsecs(1000 * ShortIntervalSecs);
   tshort.Round(SecsField, ShortIntervalSecs);
   auto delay = timeNow.MsecsUntil(tshort);

   if((delay < 0) || (delay > 1500 * ShortIntervalSecs))
   {
      Debug::SwErr(StatisticsThread_CalcFirstDelay, delay, 0);
      delay = 1000 * ShortIntervalSecs;
   }

   //  Start the first long interval for thread statistics at the next
   //  time that is at least half the distance between long intervals.
   //
   auto tlong = timeNow;
   tlong.AddMsecs(1000 * LongIntervalSecs);
   tlong.Round(MinsField, LongIntervalSecs / 60);
   auto delta = tshort.MsecsUntil(tlong);

   if((delta < 0) || (delta > 1500 * LongIntervalSecs))
   {
      Debug::SwErr(StatisticsThread_CalcFirstDelay, delta, 1);
      countdown_ = WakeupsBetweenReports;
   }
   else
   {
      countdown_ = (delta / (1000 * ShortIntervalSecs)) + 1;
   }

   wakeupTicks_ = ticksNow + Clock::MsecsToTicks(delay);
   return delay;
}

//------------------------------------------------------------------------------

fn_name StatisticsThread_Destroy = "StatisticsThread.Destroy";

void StatisticsThread::Destroy()
{
   Debug::ft(StatisticsThread_Destroy);

   Singleton< StatisticsThread >::Destroy();
}

//------------------------------------------------------------------------------

void StatisticsThread::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Thread::Display(stream, prefix, options);

   stream << prefix << "wakeupTicks : " << wakeupTicks_ << CRLF;
   stream << prefix << "countdown   : " << countdown_ << CRLF;
   stream << prefix << "delayed     : " << delayed_ << CRLF;
}

//------------------------------------------------------------------------------

fn_name StatisticsThread_Enter = "StatisticsThread.Enter";

void StatisticsThread::Enter()
{
   Debug::ft(StatisticsThread_Enter);

   auto reg = Singleton< StatisticsRegistry >::Instance();
   auto sleep = CalcFirstDelay();

   while(true)
   {
      Pause(sleep);

      //  Start the next short interval for thread statistics.
      //
      Thread::StartShortInterval();

      if(countdown_ > 0) --countdown_;

      if((countdown_ == 0) || delayed_)
      {
         //  Generate a statistics report.
         //
         auto name = StatisticsRegistry::StatsFileName() + ".txt";
         auto stream = FileThread::CreateStream();

         if(stream != nullptr)
         {
            reg->DisplayStats(*stream);
            FileThread::Spool(name, stream);
            delayed_ = false;
            auto log = Log::Create("STATISTICS REPORT GENERATED");
            if(log != nullptr) Log::Spool(log);
         }
         else
         {
            //  Setting this flag will cause repeated attempts to
            //  generate the failed statistics report.  Once the
            //  next interval is reached, however, those statistics
            //  will be rolled over again and lost, apart from their
            //  amalgamation into overall totals.
            //
            delayed_ = true;
            auto log = Log::Create("STATISTICS REPORT DELAYED");
            if(log != nullptr) Log::Spool(log);
         }

         if(countdown_ == 0)
         {
            //  At the end of the interval, start a new one in which
            //  statistics from the "current" interval become the ones
            //  from the "previous" interval and get merged into the
            //  overall statistics.
            //
            countdown_ = WakeupsBetweenReports;
            reg->StartInterval(false);
            auto log = Log::Create("STATISTICS ROLLOVER PERFORMED");
            if(log != nullptr) Log::Spool(log);
         }
      }

      //  Calculate the time when we want to wake up and sleep until then.
      //
      wakeupTicks_ += PrevToCurrTicks;
      sleep = Clock::TicksToMsecs(Clock::TicksUntil(wakeupTicks_));
   }
}

//------------------------------------------------------------------------------

void StatisticsThread::Patch(sel_t selector, void* arguments)
{
   Thread::Patch(selector, arguments);
}
}