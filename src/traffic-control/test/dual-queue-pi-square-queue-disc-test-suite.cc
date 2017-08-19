/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
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
 * Author: Shravya K.S. <shravya.ks0@gmail.com>
 *
 */

#include "ns3/test.h"
#include "ns3/dual-queue-pi-square-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

class DualQueueL4SQueueDiscTestItem : public QueueDiscItem
{
public:
  DualQueueL4SQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~DualQueueL4SQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);
  virtual bool IsL4S (void);

private:
  DualQueueL4SQueueDiscTestItem ();
  DualQueueL4SQueueDiscTestItem (const DualQueueL4SQueueDiscTestItem &);
  DualQueueL4SQueueDiscTestItem &operator = (const DualQueueL4SQueueDiscTestItem &);
};

DualQueueL4SQueueDiscTestItem::DualQueueL4SQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

DualQueueL4SQueueDiscTestItem::~DualQueueL4SQueueDiscTestItem ()
{
}

void
DualQueueL4SQueueDiscTestItem::AddHeader (void)
{
}

bool
DualQueueL4SQueueDiscTestItem::Mark (void)
{
  return true;
}

bool 
DualQueueL4SQueueDiscTestItem::IsL4S (void)
{
  return true;
}

class DualQueueClassicQueueDiscTestItem : public QueueDiscItem
{
public:
  DualQueueClassicQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~DualQueueClassicQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);
  virtual bool IsL4S (void);

private:
  DualQueueClassicQueueDiscTestItem ();
  DualQueueClassicQueueDiscTestItem (const DualQueueClassicQueueDiscTestItem &);
  DualQueueClassicQueueDiscTestItem &operator = (const DualQueueClassicQueueDiscTestItem &);
};

DualQueueClassicQueueDiscTestItem::DualQueueClassicQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

DualQueueClassicQueueDiscTestItem::~DualQueueClassicQueueDiscTestItem ()
{
}

void
DualQueueClassicQueueDiscTestItem::AddHeader (void)
{
}

bool
DualQueueClassicQueueDiscTestItem::Mark (void)
{
  return true;
}

bool 
DualQueueClassicQueueDiscTestItem::IsL4S (void)
{
  return false;
}

class DualQueuePiSquareQueueDiscTestCase : public TestCase
{
public:
  DualQueuePiSquareQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  void Enqueue (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void EnqueueWithDelay (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void Dequeue (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay (Ptr<DualQueuePiSquareQueueDisc> queue, double delay, uint32_t nPkt);
  void RunPiSquareTest (StringValue mode);
};

DualQueuePiSquareQueueDiscTestCase::DualQueuePiSquareQueueDiscTestCase ()
  : TestCase ("Sanity check on the dual queue pi square queue implementation")
{
}

void
DualQueuePiSquareQueueDiscTestCase::RunPiSquareTest (StringValue mode)
{
  uint32_t pktSize = 0;

  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;

  uint32_t qSize = 50;
  Ptr<DualQueuePiSquareQueueDisc> queue = CreateObject<DualQueuePiSquareQueueDisc> ();


  // test 1: simple enqueue/dequeue with defaults, no drops
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");

  Address dest;

  if (queue->GetMode () == DualQueuePiSquareQueueDisc::QUEUE_DISC_MODE_BYTES)
    {
      // pktSize should be same as MeanPktSize to avoid performance gap between byte and packet mode
      pktSize = 1000;
      modeSize = pktSize;
      qSize = qSize * modeSize;
    }

  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");

  Ptr<Packet> p1, p2, p3, p4, p5, p6, p7, p8;
  p1 = Create<Packet> (pktSize);
  p2 = Create<Packet> (pktSize);
  p3 = Create<Packet> (pktSize);
  p4 = Create<Packet> (pktSize);
  p5 = Create<Packet> (pktSize);
  p6 = Create<Packet> (pktSize);
  p7 = Create<Packet> (pktSize);
  p8 = Create<Packet> (pktSize);

  queue->Initialize ();
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 0 * modeSize, "There should be no packets in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem> (p1, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem> (p2, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem> (p3, dest, 0));
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem> (p4, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem> (p5, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem> (p6, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem> (p7, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem> (p8, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 8 * modeSize, "There should be eight packets in there");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 7 * modeSize, "There should be seven packets in there");
  
  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 6 * modeSize, "There should be six packet in there");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");

  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");

  
  // test 2: more data with defaults, unforced drops but no forced drops
  queue = CreateObject<DualQueuePiSquareQueueDisc> ();
  pktSize = 1000;  // pktSize != 0 because DequeueThreshold always works in bytes
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (10)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (100)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.016))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SMarkThresold", TimeValue (Seconds (0.001))), true,
                         "Verify that we can actually set the attribute L4SMarkThresold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K", UintegerValue (2)), true,
                         "Verify that we can actually set the attribute K"); 
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueDelayReference", TimeValue (Seconds (0.15))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 200, StringValue("L4S"));
  EnqueueWithDelay (queue, pktSize, 200, StringValue("Classic"));
  DequeueWithDelay (queue, 0.012, 400);
  //EnqueueWithDelay (queue, pktSize, 20, StringValue("Classic"));
  //DequeueWithDelay (queue, 10, 20);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  DualQueuePiSquareQueueDisc::Stats st = StaticCast<DualQueuePiSquareQueueDisc> (queue)->GetStats ();
  uint32_t test2ClassicMark = st.unforcedClassicMark;
  uint32_t test2L4SMark = st.unforcedL4SMark;
  NS_TEST_EXPECT_MSG_NE (test2ClassicMark, 0, "There should some unforced classic marks");
  NS_TEST_EXPECT_MSG_NE (test2L4SMark, 0, "There should some unforced l4s marks");
  NS_TEST_EXPECT_MSG_GT (test2L4SMark, test2ClassicMark, "Packets of L4S traffic should have more unforced marks than packets of Classic traffic");
  NS_TEST_EXPECT_MSG_NE (st.forcedDrop, 0, "There should be some forced drops");

  //test 3: Test by pumping only L4S traffic
  queue = CreateObject<DualQueuePiSquareQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (10)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (100)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.016))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SMarkThresold", TimeValue (Seconds (0.001))), true,
                         "Verify that we can actually set the attribute L4SMarkThresold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K", UintegerValue (2)), true,
                         "Verify that we can actually set the attribute K"); 
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueDelayReference", TimeValue (Seconds (0.15))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue("L4S"));
  DequeueWithDelay (queue, 0.012, 400); 
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<DualQueuePiSquareQueueDisc> (queue)->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.unforcedClassicDrop, 0, "There should be zero unforced classic drops since only L4S traffic is pumped ");
  NS_TEST_EXPECT_MSG_EQ (st.unforcedClassicMark, 0, "There should be zero unforced classic marks since only L4S traffic is pumped");
  NS_TEST_EXPECT_MSG_NE (st.unforcedL4SMark, 0, "There should be some L4S marks");

  //test 4: Test by pumping only Classic traffic
  queue = CreateObject<DualQueuePiSquareQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (10)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (100)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.016))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SMarkThresold", TimeValue (Seconds (0.001))), true,
                         "Verify that we can actually set the attribute L4SMarkThresold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K", UintegerValue (2)), true,
                         "Verify that we can actually set the attribute K"); 
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueDelayReference", TimeValue (Seconds (0.15))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue("Classic"));
  DequeueWithDelay (queue, 0.012, 400); 
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<DualQueuePiSquareQueueDisc> (queue)->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.unforcedClassicDrop, 0, "There should be zero unforced classic drops since packets are ECN capable ");
  NS_TEST_EXPECT_MSG_NE (st.unforcedClassicMark, 0, "There should be some unforced classic marks");
  NS_TEST_EXPECT_MSG_EQ (st.unforcedL4SMark, 0, "There should be zero L4S marks since only Classic traffic is pumped");

}


void
DualQueuePiSquareQueueDiscTestCase::Enqueue (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++)
    {
      if (trafficType.Get() == "L4S")
        {
          queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem> (Create<Packet> (size), dest, 0));
        }
      else if (trafficType.Get() == "Classic")
        {
          queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem> (Create<Packet> (size), dest, 0));
        }
    }
}

void
DualQueuePiSquareQueueDiscTestCase::EnqueueWithDelay (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  double delay = 0.01;  // enqueue packets with delay
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds (i*delay)), &DualQueuePiSquareQueueDiscTestCase::Enqueue, this, queue, size, 1, trafficType);
    }
}

void
DualQueuePiSquareQueueDiscTestCase::Dequeue (Ptr<DualQueuePiSquareQueueDisc> queue, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<QueueDiscItem> item = queue->Dequeue ();
    }
}

void
DualQueuePiSquareQueueDiscTestCase::DequeueWithDelay (Ptr<DualQueuePiSquareQueueDisc> queue, double delay, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &DualQueuePiSquareQueueDiscTestCase::Dequeue, this, queue, 1);
    }
}

void
DualQueuePiSquareQueueDiscTestCase::DoRun (void)
{
  RunPiSquareTest (StringValue ("QUEUE_DISC_MODE_PACKETS"));
  RunPiSquareTest (StringValue ("QUEUE_DISC_MODE_BYTES"));
  Simulator::Destroy ();
}

static class DualQueuePiSquareQueueDiscTestSuite : public TestSuite
{
public:
  DualQueuePiSquareQueueDiscTestSuite ()
    : TestSuite ("dual-queue-pi-square-queue-disc", UNIT)
  {
    AddTestCase (new DualQueuePiSquareQueueDiscTestCase (), TestCase::QUICK);
  }
} g_dualQueuePiSquareQueueTestSuite;
