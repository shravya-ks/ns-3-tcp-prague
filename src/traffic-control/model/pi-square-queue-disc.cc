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

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "pi-square-queue-disc.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PiSquareQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (PiSquareQueueDisc);

TypeId PiSquareQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PiSquareQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<PiSquareQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_DISC_MODE_PACKETS),
                   MakeEnumAccessor (&PiSquareQueueDisc::SetMode),
                   MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
                                    QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&PiSquareQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("A",
                   "Value of alpha",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&PiSquareQueueDisc::m_a),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Value of beta",
                   DoubleValue (1.25),
                   MakeDoubleAccessor (&PiSquareQueueDisc::m_b),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Tupdate",
                   "Time period to calculate drop probability",
                   TimeValue (Seconds (0.03)),
                   MakeTimeAccessor (&PiSquareQueueDisc::m_tUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("Supdate",
                   "Start time of the update timer",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PiSquareQueueDisc::m_sUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (25),
                   MakeUintegerAccessor (&PiSquareQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DequeueThreshold",
                   "Minimum queue size in bytes before dequeue rate is measured",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&PiSquareQueueDisc::m_dqThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueDelayReference",
                   "Desired queue delay",
                   TimeValue (Seconds (0.02)),
                   MakeTimeAccessor (&PiSquareQueueDisc::m_qDelayRef),
                   MakeTimeChecker ())
    .AddAttribute ("CoupledAqm",
                   "True to enable Coupled AQM Functionality",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PiSquareQueueDisc::SetCoupledAqm),
                   MakeBooleanChecker ())
    .AddAttribute ("UseEcn",
                   "True to enable ECN (packets are marked instead of being dropped)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PiSquareQueueDisc::m_useEcn),
                   MakeBooleanChecker ())
  ;

  return tid;
}

PiSquareQueueDisc::PiSquareQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent = Simulator::Schedule (m_sUpdate, &PiSquareQueueDisc::CalculateP, this);
}

PiSquareQueueDisc::~PiSquareQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
PiSquareQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent);
  QueueDisc::DoDispose ();
}

void
PiSquareQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

PiSquareQueueDisc::QueueDiscMode
PiSquareQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
PiSquareQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

void
PiSquareQueueDisc::SetCoupledAqm (bool coupledAqm)
{
  NS_LOG_FUNCTION (this);
  m_coupledAqm = coupledAqm;
  // For Coupled AQM functionality, ECN must be enabled
  if (coupledAqm)
    {
      m_useEcn = true;
    }
}

uint32_t
PiSquareQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      return GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
    {
      return GetInternalQueue (0)->GetNPackets ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown PI Square mode.");
    }
}

PiSquareQueueDisc::Stats
PiSquareQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

Time
PiSquareQueueDisc::GetQueueDelay (void)
{
  NS_LOG_FUNCTION (this);
  return m_qDelay;
}

double
PiSquareQueueDisc::GetDropProb (void)
{
  NS_LOG_FUNCTION (this);
  return m_dropProb;
}

int64_t
PiSquareQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
PiSquareQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  uint32_t nQueued = GetQueueSize ();

  if ((GetMode () == QUEUE_DISC_MODE_PACKETS && nQueued >= m_queueLimit)
      || (GetMode () == QUEUE_DISC_MODE_BYTES && nQueued + item->GetSize () > m_queueLimit))
    {
      // Drops due to queue limit: reactive
      Drop (item);
      m_stats.forcedDrop++;
      return false;
    }
  else if (DropEarly (item, nQueued))
    {
      // Early probability drop: proactive
      if (!m_useEcn || !item->Mark ())
        {
          Drop (item);
          m_stats.unforcedDrop++;
          return false;
        }
      m_stats.unforcedMark++; 
    }

  // No drop
  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::Drop is called by the internal queue
  // because QueueDisc::AddInternalQueue sets the drop callback

  NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

  return retval;
}

void
PiSquareQueueDisc::InitializeParams (void)
{
  // Initially queue is empty so variables are initialize to zero except m_dqCount
  m_inMeasurement = false;
  m_dqCount = -1;
  m_dropProb = 0;
  m_avgDqRate = 0.0;
  m_dqStart = 0;
  m_qDelayOld = Time (Seconds (0));
  m_stats.forcedDrop = 0;
  m_stats.unforcedDrop = 0;
}

bool PiSquareQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
{
  NS_LOG_FUNCTION (this << item << qSize);

  double p = m_dropProb;

  uint32_t packetSize = item->GetSize ();

  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      p = p * packetSize / m_meanPktSize;
    }
  bool earlyDrop = true;
  double u =  m_uv->GetValue ();

  if (GetMode () == QUEUE_DISC_MODE_BYTES && qSize <= 2 * m_meanPktSize)
    {
      return false;
    }
  else if (GetMode () == QUEUE_DISC_MODE_PACKETS && qSize <= 2)
    {
      return false;
    }

  if (m_coupledAqm && item->IsL4S ())
    {
       // Apply linear drop probability
       if (u > p)
         {
           earlyDrop = false;
         }
    }
  else 
    {
       // Apply the squared drop probability
       if (u > p * p)
        {
          earlyDrop = false;
        }
    }
  if (!earlyDrop)
    {
      return false;
    }

  return true;
}

void PiSquareQueueDisc::CalculateP ()
{
  NS_LOG_FUNCTION (this);
  Time qDelay;
  double p = 0.0;
  bool missingInitFlag = false;
  if (m_avgDqRate > 0)
    {
      qDelay = Time (Seconds (GetInternalQueue (0)->GetNBytes () / m_avgDqRate));
    }
  else
    {
      qDelay = Time (Seconds (0));
      missingInitFlag = true;
    }

  m_qDelay = qDelay;

  // Calculate the drop probability
  p = m_a * (qDelay.GetSeconds () - m_qDelayRef.GetSeconds ()) + m_b * (qDelay.GetSeconds () - m_qDelayOld.GetSeconds ());
  p += m_dropProb;

  // For non-linear drop in prob

  if (qDelay.GetSeconds () == 0 && m_qDelayOld.GetSeconds () == 0)
    {
      p *= 0.98;
    }

  m_dropProb = (p > 0) ? p : 0;

  if ( (qDelay.GetSeconds () < 0.5 * m_qDelayRef.GetSeconds ()) && (m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb == 0) && !missingInitFlag )
    {
      m_dqCount = -1;
      m_avgDqRate = 0.0;
    }

  m_qDelayOld = qDelay;
  m_rtrsEvent = Simulator::Schedule (m_tUpdate, &PiSquareQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem>
PiSquareQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<QueueDiscItem> item = StaticCast<QueueDiscItem> (GetInternalQueue (0)->Dequeue ());
  double now = Simulator::Now ().GetSeconds ();
  uint32_t pktSize = item->GetSize ();

  // if not in a measurement cycle and the queue has built up to dq_threshold,
  // start the measurement cycle

  if ( (GetInternalQueue (0)->GetNBytes () >= m_dqThreshold) && (!m_inMeasurement) )
    {
      m_dqStart = now;
      m_dqCount = 0;
      m_inMeasurement = true;
    }

  if (m_inMeasurement)
    {
      m_dqCount += pktSize;

      // done with a measurement cycle
      if (m_dqCount >= m_dqThreshold)
        {

          double tmp = now - m_dqStart;

          if (tmp > 0)
            {
              if (m_avgDqRate == 0)
                {
                  m_avgDqRate = m_dqCount / tmp;
                }
              else
                {
                  m_avgDqRate = (0.5 * m_avgDqRate) + (0.5 * (m_dqCount / tmp));
                }
            }

          // restart a measurement cycle if there is enough data
          if (GetInternalQueue (0)->GetNBytes () > m_dqThreshold)
            {
              m_dqStart = now;
              m_dqCount = 0;
              m_inMeasurement = true;
            }
          else
            {
              m_dqCount = 0;
              m_inMeasurement = false;
            }
        }
    }

  return item;
}

Ptr<const QueueDiscItem>
PiSquareQueueDisc::DoPeek () const
{
  NS_LOG_FUNCTION (this);
  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<const QueueDiscItem> item = StaticCast<const QueueDiscItem> (GetInternalQueue (0)->Peek ());

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return item;
}

bool
PiSquareQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("PiSquareQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("PiSquareQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create a DropTail queue
      Ptr<InternalQueue> queue = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      if (m_mode == QUEUE_DISC_MODE_PACKETS)
        {
          queue->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue);
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("PiSquareQueueDisc needs 1 internal queue");
      return false;
    }

  if ((GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES) ||
      (GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
     {
       NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the PiSquareQueueDisc");
       return false;
     }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal queue is less than the queue disc limit");
      return false;
    }

  return true;
}

} //namespace ns3
