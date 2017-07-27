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

#include "ns3/test.h"
#include "ns3/pi-square-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

class PiSquareQueueDiscTestItem : public QueueDiscItem
{
public:
  PiSquareQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~PiSquareQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);
  virtual bool IsScalable(void);

private:
  PiSquareQueueDiscTestItem ();
  PiSquareQueueDiscTestItem (const PiSquareQueueDiscTestItem &);
  PiSquareQueueDiscTestItem &operator = (const PiSquareQueueDiscTestItem &);
};

PiSquareQueueDiscTestItem::PiSquareQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

PiSquareQueueDiscTestItem::~PiSquareQueueDiscTestItem ()
{
}

void
PiSquareQueueDiscTestItem::AddHeader (void)
{
}

bool
PiSquareQueueDiscTestItem::Mark (void)
{
  return false;
}

bool
PiSquareQueueDiscTestItem::IsScalable (void)
{
  return false;
}

class ScalableQueueDiscTestItem : public QueueDiscItem
{
public:
  ScalableQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~ScalableQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);
  virtual bool IsScalable (void);

private:
  ScalableQueueDiscTestItem ();
  ScalableQueueDiscTestItem (const ScalableQueueDiscTestItem &);
  ScalableQueueDiscTestItem &operator = (const ScalableQueueDiscTestItem &);
};

ScalableQueueDiscTestItem::ScalableQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

ScalableQueueDiscTestItem::~ScalableQueueDiscTestItem ()
{
}

void
ScalableQueueDiscTestItem::AddHeader (void)
{
}

bool
ScalableQueueDiscTestItem::Mark (void)
{
  return true;
}

bool 
ScalableQueueDiscTestItem::IsScalable (void)
{
  return true;
}

class ClassicQueueDiscTestItem : public QueueDiscItem
{
public:
  ClassicQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~ClassicQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);
  virtual bool IsScalable (void);

private:
  ClassicQueueDiscTestItem ();
  ClassicQueueDiscTestItem (const ClassicQueueDiscTestItem &);
  ClassicQueueDiscTestItem &operator = (const ClassicQueueDiscTestItem &);
};

ClassicQueueDiscTestItem::ClassicQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

ClassicQueueDiscTestItem::~ClassicQueueDiscTestItem ()
{
}

void
ClassicQueueDiscTestItem::AddHeader (void)
{
}

bool
ClassicQueueDiscTestItem::Mark (void)
{
  return true;
}

bool 
ClassicQueueDiscTestItem::IsScalable (void)
{
  return false;
}

class PiSquareQueueDiscTestCase : public TestCase
{
public:
  PiSquareQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  void Enqueue (Ptr<PiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void EnqueueWithDelay (Ptr<PiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void Dequeue (Ptr<PiSquareQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay (Ptr<PiSquareQueueDisc> queue, double delay, uint32_t nPkt);
  void RunPiSquareTest (StringValue mode);
};

PiSquareQueueDiscTestCase::PiSquareQueueDiscTestCase ()
  : TestCase ("Sanity check on the pi square queue implementation")
{
}

void
PiSquareQueueDiscTestCase::RunPiSquareTest (StringValue mode)
{
  uint32_t pktSize = 0;

  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;

  uint32_t qSize = 300;
  Ptr<PiSquareQueueDisc> queue = CreateObject<PiSquareQueueDisc> ();


  // test 1: simple enqueue/dequeue with defaults, no drops
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");

  Address dest;

  if (queue->GetMode () == PiSquareQueueDisc::QUEUE_DISC_MODE_BYTES)
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
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p1, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p2, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p3, dest, 0));
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p4, dest, 0));
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p5, dest, 0));
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p6, dest, 0));
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p7, dest, 0));
  queue->Enqueue (Create<PiSquareQueueDiscTestItem> (p8, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 8 * modeSize, "There should be eight packets in there");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 7 * modeSize, "There should be seven packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p1->GetUid (), "was this the first packet ?");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 6 * modeSize, "There should be six packet in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p2->GetUid (), "Was this the second packet ?");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p3->GetUid (), "Was this the third packet ?");

  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");


  // test 2: more data with defaults, unforced drops but no forced drops
  queue = CreateObject<PiSquareQueueDisc> ();
  pktSize = 1000;  // pktSize != 0 because DequeueThreshold always works in bytes
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (0.125)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (1.25)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("DequeueThreshold", UintegerValue (10000)), true,
                         "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueDelayReference", TimeValue (Seconds (0.02))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue("NonDualQ"));
  DequeueWithDelay (queue, 0.012, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  PiSquareQueueDisc::Stats st = StaticCast<PiSquareQueueDisc> (queue)->GetStats ();
  uint32_t test2 = st.unforcedDrop;
  NS_TEST_EXPECT_MSG_NE (test2, 0, "There should some unforced drops");
  NS_TEST_EXPECT_MSG_EQ (st.forcedDrop, 0, "There should zero forced drops");


  // test 3: same as test 2, but with higher QueueDelayReference
  queue = CreateObject<PiSquareQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (0.125)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (1.25)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("DequeueThreshold", UintegerValue (10000)), true,
                         "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueDelayReference", TimeValue (Seconds (0.08))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue("NonDualQ"));
  DequeueWithDelay (queue, 0.012, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<PiSquareQueueDisc> (queue)->GetStats ();
  uint32_t test3 = st.unforcedDrop;
  NS_TEST_EXPECT_MSG_LT (test3, test2, "Test 3 should have less unforced drops than test 2");
  NS_TEST_EXPECT_MSG_EQ (st.forcedDrop, 0, "There should zero forced drops");


  // test 4: same as test 2, but with lesser dequeue rate
  queue = CreateObject<PiSquareQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("A", DoubleValue (0.125)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("B", DoubleValue (1.25)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("DequeueThreshold", UintegerValue (10000)), true,
                         "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueDelayReference", TimeValue (Seconds (0.02))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue("NonDualQ"));
  DequeueWithDelay (queue, 0.015, 400); // delay between two successive dequeue events is increased
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<PiSquareQueueDisc> (queue)->GetStats ();
  uint32_t test4 = st.unforcedDrop;
  NS_TEST_EXPECT_MSG_GT (test4, test2, "Test 4 should have more unforced drops than test 2");  
  NS_TEST_EXPECT_MSG_EQ (st.forcedDrop, 0, "There should zero forced drops");

  // test 5: Tests for DualQ Framework
  Ptr<PiSquareQueueDisc> queue1 = CreateObject<PiSquareQueueDisc> ();
  Ptr<PiSquareQueueDisc> queue2 = CreateObject<PiSquareQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("A", DoubleValue (0.125)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("A", DoubleValue (0.125)), true,
                         "Verify that we can actually set the attribute A");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("B", DoubleValue (1.25)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("B", DoubleValue (1.25)), true,
                         "Verify that we can actually set the attribute B");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
   NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("Supdate", TimeValue (Seconds (0.0))), true,
                         "Verify that we can actually set the attribute Supdate");
   NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("DequeueThreshold", UintegerValue (10000)), true,
                         "Verify that we can actually set the attribute DequeueThreshold");
   NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("QueueDelayReference", TimeValue (Seconds (0.02))), true,
                         "Verify that we can actually set the attribute QueueDelayReference");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("Tupdate", TimeValue (Seconds (0.03))), true,
                         "Verify that we can actually set the attribute Tupdate");
  NS_TEST_EXPECT_MSG_EQ (queue1->SetAttributeFailSafe ("UseDualQ", BooleanValue (true)), true,
                         "Verify that we can actually set the attribute UseDualQ");
  NS_TEST_EXPECT_MSG_EQ (queue2->SetAttributeFailSafe ("UseDualQ", BooleanValue (true)), true,
                         "Verify that we can actually set the attribute UseDualQ");

  BooleanValue useEcn1;
  BooleanValue useEcn2;
  queue1->GetAttribute ("UseEcn", useEcn1);
  NS_TEST_EXPECT_MSG_EQ (useEcn1.Get(), true,
                         "Verify that the attribute UseEcn of queue1 is getting set to true when UseDualQ is true");
  queue2->GetAttribute ("UseEcn", useEcn2);
  NS_TEST_EXPECT_MSG_EQ (useEcn2.Get(), true,
                         "Verify that the attribute UseEcn of queue2 is getting set to true when UseDualQ is true");
  queue1->Initialize ();
  queue2->Initialize ();
  EnqueueWithDelay (queue1, pktSize, 400, StringValue("Scalable"));
  EnqueueWithDelay (queue2, pktSize, 400, StringValue("Classic"));
  DequeueWithDelay (queue1, 0.015, 400); 
  DequeueWithDelay (queue2, 0.015, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  PiSquareQueueDisc::Stats st1 = StaticCast<PiSquareQueueDisc> (queue1)->GetStats ();
  PiSquareQueueDisc::Stats st2 = StaticCast<PiSquareQueueDisc> (queue2)->GetStats ();
  uint32_t mark1 = st1.unforcedMark;
  uint32_t mark2 = st2.unforcedMark;
  NS_TEST_EXPECT_MSG_GT (mark1, mark2, "Packets of Scalable traffic should have more unforced marks than packets of Classic traffic");  
  NS_TEST_EXPECT_MSG_EQ (st1.unforcedDrop, 0, "There should be zero unforced drops for packets of Scalable traffic");
  NS_TEST_EXPECT_MSG_EQ (st2.unforcedDrop, 0, "There should be zero unforced drops for packets of Classic traffic when its ECN capable");

}

void
PiSquareQueueDiscTestCase::Enqueue (Ptr<PiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++)
    {
      //NS_TEST_EXPECT_MSG_EQ (trafficType.Get(), "Scalable", "Works");
      if (trafficType.Get() == "Scalable")
        {
          NS_TEST_EXPECT_MSG_EQ (trafficType.Get(), "Scalable", "Works");
          queue->Enqueue (Create<ScalableQueueDiscTestItem> (Create<Packet> (size), dest, 0));
        }
      else if (trafficType.Get() == "Classic")
        {
          queue->Enqueue (Create<ClassicQueueDiscTestItem> (Create<Packet> (size), dest, 0));
        }
      else
        {
          queue->Enqueue (Create<PiSquareQueueDiscTestItem> (Create<Packet> (size), dest, 0));
        }
    }
}

void
PiSquareQueueDiscTestCase::EnqueueWithDelay (Ptr<PiSquareQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  double delay = 0.01;  // enqueue packets with delay
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &PiSquareQueueDiscTestCase::Enqueue, this, queue, size, 1, trafficType);
    }
}

void
PiSquareQueueDiscTestCase::Dequeue (Ptr<PiSquareQueueDisc> queue, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<QueueDiscItem> item = queue->Dequeue ();
    }
}

void
PiSquareQueueDiscTestCase::DequeueWithDelay (Ptr<PiSquareQueueDisc> queue, double delay, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &PiSquareQueueDiscTestCase::Dequeue, this, queue, 1);
    }
}

void
PiSquareQueueDiscTestCase::DoRun (void)
{
  RunPiSquareTest (StringValue ("QUEUE_DISC_MODE_PACKETS"));
  RunPiSquareTest (StringValue ("QUEUE_DISC_MODE_BYTES"));
  Simulator::Destroy ();
}

static class PiSquareQueueDiscTestSuite : public TestSuite
{
public:
  PiSquareQueueDiscTestSuite ()
    : TestSuite ("pi-square-queue-disc", UNIT)
  {
    AddTestCase (new PiSquareQueueDiscTestCase (), TestCase::QUICK);
  }
} g_piSquareQueueTestSuite;
