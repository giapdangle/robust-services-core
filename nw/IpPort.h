//==============================================================================
//
//  IpPort.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef IPPORT_H_INCLUDED
#define IPPORT_H_INCLUDED

#include "Protected.h"
#include <cstddef>
#include <iosfwd>
#include <memory>
#include "NwTypes.h"
#include "Q1Link.h"

namespace NodeBase
{
   class IpPortStats;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  An IP port that supports a service (an application protocol).
//
class IpPort : public Protected
{
   friend class InputHandler;
public:
   //  Deregisters the port.  Shuts down any I/O thread and deletes any input
   //  handler.  The I/O thread must delete its socket(s) when it shuts down.
   //  Virtual to allow subclassing.
   //
   virtual ~IpPort();

   //  Returns the IP port.
   //
   ipport_t GetPort() const { return port_; }

   //  Returns the port's service.
   //
   IpService* GetService() const { return service_; }

   //  Returns the port's input handler.
   //
   InputHandler* GetHandler() const { return handler_.get(); }

   //  Returns the port's I/O thread.
   //
   IoThread* GetThread() const { return thread_; }

   //  Sets (or clears, if nullptr) the port's I/O thread.
   //
   void SetThread(IoThread* thread);

   //  Returns the port's socket.
   //
   SysSocket* GetSocket() const { return socket_; }

   //  Sets (or clears, if nullptr) the port's socket.  If the socket is valid,
   //  the port must already have an input handler and I/O thread.
   //
   bool SetSocket(SysSocket* socket);

   //  Creates a socket for an application instance.  rxSize and txSize specify
   //  the size of the incoming and outgoing buffers, respectively.  The socket
   //  is registered with the port's I/O thread, which uses Poll() to receive
   //  its messages.  The default version returns nullptr and is overridden by
   //  a port that supports a socket for each application instance.
   //
   virtual SysSocket* CreateAppSocket(size_t rxSize, size_t txSize);

   //  Invoked after COUNT bytes were received.
   //
   void BytesRcvd(size_t count) const;

   //  Invoked after COUNT bytes were sent.
   //
   void BytesSent(size_t count) const;

   //  Invoked after COUNT receive operations were performed before yielding.
   //
   void RecvsInSequence(size_t count) const;

   //  Invoked when an incoming message is discarded because it is invalid.
   //
   void InvalidDiscarded() const;

   //  Invoked when ingress work is discarded because of overload controls.
   //
   void IngressDiscarded() const;

   //  Invoked when the array of sockets used for polling is full, preventing
   //  another socket from being added.
   //
   void PollArrayOverflow() const;

   //  Returns the number of messages discarded by overload controls during
   //  the current statistics interval.
   //
   size_t Discards() const;

   //  Displays statistics.  May be overridden to include pool-specific
   //  statistics, but the base class version must be invoked.
   //
   virtual void DisplayStats(std::ostream& stream) const;

   //  Returns the offset to link_.
   //
   static ptrdiff_t LinkDiff();

   //  Overridden for restarts.
   //
   virtual void Startup(RestartLevel level) override;

   //  Overridden for restarts.
   //
   virtual void Shutdown(RestartLevel level) override;

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;
protected:
   //  Assigns SERVICE to PORT, registering it with IpPortRegistry.  If PORT
   //  is available, this eventually results in the creation of an I/O thread
   //  that runs the protocol (e.g. UDP) specified by SERVICE, in the faction
   //  (e.g. PayloadFaction) also specified by SERVICE.  When the I/O thread
   //  is entered, it allocates a socket, receives messages on the port, and
   //  passes them to an input handler created by SERVICE.  Protected because
   //  this class is virtual.
   //
   IpPort(ipport_t port, IpService* service);

   //  Creates an I/O thread for the port.  The default version
   //  generates a fatal log and must be overridden by a subclass
   //  that has an input handler.
   //
   virtual IoThread* CreateIoThread();
private:
   //  Sets the port's input handler.  If the port does not have
   //  an I/O thread, it is created.
   //
   bool BindHandler(InputHandler& handler);

   //  Clears the port's input handler.  If the port has an I/O
   //  thread, it is shut down.
   //
   void UnbindHandler(const InputHandler& handler);

   //  Overridden to prohibit copying.
   //
   IpPort(const IpPort& that);
   void operator=(const IpPort& that);

   //  The next entry in IpPortRegistry.
   //
   Q1Link link_;

   //  The port number associated with this entry.
   //
   ipport_t port_;

   //  The port's service.
   //
   IpService* service_;

   //  The port's input handler.
   //
   std::unique_ptr< InputHandler > handler_;

   //  The port's I/O thread.
   //
   IoThread* thread_;

   //  The port's socket.
   //
   SysSocket* socket_;

   //  The port's statistics.
   //
   std::unique_ptr< IpPortStats > stats_;
};
}
#endif