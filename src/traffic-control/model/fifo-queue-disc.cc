/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universita' degli Studi di Napoli Federico II
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:  Stefano Avallone <stavallo@unina.it>
 */
#include "ns3/log.h"
#include "fifo-queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"

#include "ns3/core-module.h"
#include "ns3/tcp-header.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-header.h"
#include "ns3/packet.h"
#include "ns3/internet-module.h"
#include "ns3/queue-disc.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FifoQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (FifoQueueDisc);

TypeId FifoQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FifoQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<FifoQueueDisc> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
  ;
  return tid;
}

FifoQueueDisc::FifoQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
}

FifoQueueDisc::~FifoQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}
// static ns3::GlobalValue prerttglobalqueue ("prerttglobal", "prertt",
//                                    ns3::DoubleValue(0),
//                                    ns3::MakeDoubleChecker<double> ()); 
void FifoQueueDisc::wrapperyyq(Ptr<QueueDiscItem> item){
    // GetInternalQueue (0)->Enqueue (item);
    m_devQueueIface->GetTxQueue (item->GetTxQueueIndex ())->Wake();
}

bool
FifoQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  DoubleValue currentprertt;
  GlobalValue::GetValueByNameFailSafe ("prerttglobal", currentprertt);
  // PointerValue ptr;
  // GlobalValue::GetValueByNameFailSafe("apwindow",ptr);
  // UintegerValue val;
  // Ptr<TcpL4Protocol> ptryyq = ptr.Get<TcpL4Protocol> ();
  // uint32_t rxWindowSize = ptryyq->GetRxWindowSize ();
  // std::cout<<rxWindowSize<<"\n";


  bool retval;
  TcpHeader yyqtcp;
  item->GetPacket()->PeekHeader(yyqtcp);
  uint32_t initcwnd = 8388608;
  uint32_t cwndplus = 16384;
  // header.GetSource()<<" Received by "<<header.GetDestination()std::endl;
  if (GetCurrentSize () + item > GetMaxSize ())
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
      return false;
    }
  if(currentprertt.Get()>0.17){ 
    if(yyqtcp.GetFlags()==16){
      // Simulator::Schedule(Seconds(0.001),&FifoQueueDisc::wrapperyyq,this,item);
      // retval = GetInternalQueue (0)->Enqueue (item);
      m_devQueueIface->GetTxQueue (item->GetTxQueueIndex ())->Stop();  

      // m_devQueueIface->GetTxQueue (item->GetTxQueueIndex ())->Wake();  
      Simulator::Schedule(Seconds(0.015),&FifoQueueDisc::wrapperyyq,this,item);
       initcwnd = 8388608;
      Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(initcwnd));
      // FifoQueueDisc::wrapperyyq(item);
      // retval = true;
      retval = GetInternalQueue (0)->Enqueue (item);
    } 
  }
  else if(currentprertt.Get()<0.09){
    uint32_t cwnd = cwndplus+initcwnd;
    initcwnd = cwnd;
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(cwnd));
      retval = GetInternalQueue (0)->Enqueue (item);
  }
  else{
    
      retval = GetInternalQueue (0)->Enqueue (item);
       
  }
 

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return retval;
}

Ptr<QueueDiscItem>
FifoQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

Ptr<const QueueDiscItem>
FifoQueueDisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

bool
FifoQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDisc needs no packet filter");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("FifoQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

void
FifoQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
