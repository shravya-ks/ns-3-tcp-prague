/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Trinity College Dublin
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
 * Authors: Rohit P. Tahiliani <rohit.tahil@gmail.com>
 *
 */

#include "math.h"
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/object-factory.h"
#include "ns3/string.h"
#include "dual-queue-pi-square-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#define min(a,b) ((a) < (b) ? (a) : (b))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DualQueuePiSquareQueueDisc");

class DualQueuePiSquareTimestampTag : public Tag
{
public:
  DualQueuePiSquareTimestampTag ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Gets the Tag creation time
   * @return the time object stored in the tag
   */
  Time GetTxTime (void) const;
private:
  uint64_t m_creationTime; //!< Tag creation time
};

DualQueuePiSquareTimestampTag::DualQueuePiSquareTimestampTag ()
  : m_creationTime (Simulator::Now ().GetTimeStep ())
{
}

TypeId
DualQueuePiSquareTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQueuePiSquareTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<DualQueuePiSquareTimestampTag> ()
    .AddAttribute ("CreationTime",
                   "The time at which the timestamp was created",
                   StringValue ("0.0s"),
                   MakeTimeAccessor (&DualQueuePiSquareTimestampTag::GetTxTime),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
DualQueuePiSquareTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DualQueuePiSquareTimestampTag::GetSerializedSize (void) const
{
  return 8;
}
void
DualQueuePiSquareTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_creationTime);
}
void
DualQueuePiSquareTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = i.ReadU64 ();
}
void
DualQueuePiSquareTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime=" << m_creationTime;
}
Time
DualQueuePiSquareTimestampTag::GetTxTime (void) const
{
  return TimeStep (m_creationTime);
}

NS_OBJECT_ENSURE_REGISTERED (DualQueuePiSquareQueueDisc);

TypeId DualQueuePiSquareQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQueuePiSquareQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DualQueuePiSquareQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_DISC_MODE_PACKETS),
                   MakeEnumAccessor (&DualQueuePiSquareQueueDisc::SetMode),
                   MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
                                    QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&DualQueuePiSquareQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("A",
                   "Value of alpha",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&DualQueuePiSquareQueueDisc::m_alpha),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Value of beta",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&DualQueuePiSquareQueueDisc::m_beta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Tupdate",
                   "Time period to calculate drop probability",
                   TimeValue (Seconds (0.16)),
                   MakeTimeAccessor (&DualQueuePiSquareQueueDisc::m_tUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("Supdate",
                   "Start time of the update timer",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&DualQueuePiSquareQueueDisc::m_sUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (25),
                   MakeUintegerAccessor (&DualQueuePiSquareQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ClassicQueueDelayReference",
                   "Desired queue delay of Classic traffic",
                   TimeValue (Seconds (0.15)),
                   MakeTimeAccessor (&DualQueuePiSquareQueueDisc::m_classicQueueDelayRef),
                   MakeTimeChecker ())
    .AddAttribute ("L4SMarkThresold",
                   "L4S marking threshold in Time",
                   TimeValue (Seconds (0.001)),
                   MakeTimeAccessor (&DualQueuePiSquareQueueDisc::m_l4sThreshold),
                   MakeTimeChecker ())
    .AddAttribute ("K",
                   "Coupling factor",
                   UintegerValue (2),
                   MakeUintegerAccessor (&DualQueuePiSquareQueueDisc::m_k),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxClassicProb",
                   "Maximum Classic drop/mark probability",
                   DoubleValue (0.25),
                   MakeDoubleAccessor (&DualQueuePiSquareQueueDisc::m_maxClassicProb),
                   MakeDoubleChecker<double> ())
  ;

  return tid;
}

DualQueuePiSquareQueueDisc::DualQueuePiSquareQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent = Simulator::Schedule (m_sUpdate, &DualQueuePiSquareQueueDisc::CalculateP, this);
}

DualQueuePiSquareQueueDisc::~DualQueuePiSquareQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
DualQueuePiSquareQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent);
  QueueDisc::DoDispose ();
}

void
DualQueuePiSquareQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

DualQueuePiSquareQueueDisc::QueueDiscMode
DualQueuePiSquareQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
DualQueuePiSquareQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

uint32_t
DualQueuePiSquareQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      return (GetInternalQueue (0)->GetNBytes () + GetInternalQueue (1)->GetNBytes ());
    }
  else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
    {
      return (GetInternalQueue (0)->GetNPackets () + GetInternalQueue (0)->GetNPackets ());
    }
  else
    {
      NS_ABORT_MSG ("Unknown Dual Queue PI Square mode.");
    }
}

DualQueuePiSquareQueueDisc::Stats
DualQueuePiSquareQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

Time
DualQueuePiSquareQueueDisc::GetQueueDelay (void)
{
  NS_LOG_FUNCTION (this);
  return m_qDelay;
}

int64_t
DualQueuePiSquareQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
DualQueuePiSquareQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  int queueNumber;

  //attach arrival time to packet
  Ptr<Packet> p = item->GetPacket ();
  DualQueuePiSquareTimestampTag tag;
  p->AddPacketTag (tag);
  if ((GetMode () == QUEUE_DISC_MODE_PACKETS && GetQueueSize () > m_queueLimit)
      || (GetMode () == QUEUE_DISC_MODE_BYTES && GetQueueSize () > m_queueLimit))
    {
      // Drops due to queue limit
      Drop (item);
      m_stats.forcedDrop++;
      return false;
    }
  else
    {
      if (item->IsScalable ())
        {
          queueNumber = 1;
          m_l4sQueueTime = Simulator::Now ();
        }
      else
        {
          queueNumber = 0;
          m_classicQueueTime = Simulator::Now ();
        }
    }

  bool retval = GetInternalQueue (queueNumber)->Enqueue (item);
  NS_LOG_LOGIC ("Number packets in queue-number " << queueNumber << ": " << GetInternalQueue (queueNumber)->GetNPackets ());
  return retval;
}

void
DualQueuePiSquareQueueDisc::InitializeParams (void)
{
  m_tShift = 2 * m_classicQueueDelayRef;
  m_alphaU = m_alpha * m_tUpdate.GetSeconds ();
  m_betaU = m_beta * m_tUpdate.GetSeconds ();
  m_maxL4SProb  = min (m_k * sqrt (m_maxClassicProb), 1);
  m_minL4SLength = 2 * m_meanPktSize;
  m_dropProb = 0.0;
  m_qDelayOld = Time (Seconds (0));
  m_stats.forcedDrop = 0;
  m_stats.unforcedDrop = 0;
}


void DualQueuePiSquareQueueDisc::CalculateP ()
{
  NS_LOG_FUNCTION (this);
  //Use queuing time of first-in Classic packet
  Time qDelay = m_classicQueueTime;
  double p = m_alphaU * (qDelay.GetSeconds () - m_classicQueueDelayRef.GetSeconds ()) +
    m_betaU * (qDelay.GetSeconds () - m_qDelayOld.GetSeconds ());
  p += m_dropProb;
  m_l4sDropProb = p * m_k;
  m_classicDropProb = p * p;
  m_qDelayOld = qDelay;
  m_dropProb = (p > 0) ? p : 0;
  m_rtrsEvent = Simulator::Schedule (m_tUpdate, &DualQueuePiSquareQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem>
DualQueuePiSquareQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  if (m_l4sQueueTime.GetSeconds () + m_tShift.GetSeconds () >= m_classicQueueTime.GetSeconds ())
    {
      Ptr<QueueDiscItem> item = GetInternalQueue (1)->Dequeue ();
      DualQueuePiSquareTimestampTag tag;
      item->GetPacket ()->RemovePacketTag (tag);
      if ((Simulator::Now () - tag.GetTxTime () > m_l4sThreshold && GetInternalQueue (1)->GetNBytes () > m_minL4SLength) || (m_l4sDropProb > m_uv->GetValue ()))
        {
          item->Mark ();
        }
      return item;
    }

  else
    {
      Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
      if (m_classicDropProb / m_k >  m_uv->GetValue ())
        {
          if (!item->Mark ())
            {
              Drop (item);
              //todo
            }
          return item;
        }
    }
  return 0;
}

Ptr<const QueueDiscItem>
DualQueuePiSquareQueueDisc::DoPeek () const
{
  NS_LOG_FUNCTION (this);
  Ptr<const QueueDiscItem> item;

  for (uint32_t i = 0; i < GetNInternalQueues (); i++)
    {
      if ((item = GetInternalQueue (i)->Peek ()) != 0)
        {
          NS_LOG_LOGIC ("Peeked from queue number " << i << ": " << item);
          NS_LOG_LOGIC ("Number packets queue number " << i << ": " << GetInternalQueue (i)->GetNPackets ());
          NS_LOG_LOGIC ("Number bytes queue number " << i << ": " << GetInternalQueue (i)->GetNBytes ());
          return item;
        }
    }

  NS_LOG_LOGIC ("Queue empty");
  return item;
}

bool
DualQueuePiSquareQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("DualQueuePiSquareQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("DualQueuePiSquareQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create 2 DropTail queues
      Ptr<InternalQueue> queue1 = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      Ptr<InternalQueue> queue2 = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      if (m_mode == QUEUE_DISC_MODE_PACKETS)
        {
          queue1->SetMaxPackets (m_queueLimit);
          queue2->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue1->SetMaxBytes (m_queueLimit);
          queue2->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue1);
      AddInternalQueue (queue2);
    }

  if (GetNInternalQueues () != 2)
    {
      NS_LOG_ERROR ("DualQueuePiSquareQueueDisc needs 2 internal queue");
      return false;
    }

  if ((GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode of the provided Classic queue does not match the mode set on the DualQueuePiSquareQueueDisc");
      return false;
    }

  if ((GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode of the provided L4S queue does not match the mode set on the DualQueuePiSquareQueueDisc");
      return false;
    }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal Classic queue is less than the queue disc limit");
      return false;
    }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (1)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (1)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal L4S queue is less than the queue disc limit");
      return false;
    }

  return true;
}

} //namespace ns3
