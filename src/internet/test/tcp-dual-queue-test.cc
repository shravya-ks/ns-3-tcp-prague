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
 * Authors: Shravya Ks <shravya.ks0@gmail.com>
 *
 */
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "../model/ipv4-end-point.h"
#include "../model/ipv6-end-point.h"
#include "tcp-general-test.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "tcp-error-model.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/tcp-dctcp.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpDualQueueTestSuite");

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief checks if ECT1 and ECT0 are set correctly for Classic and L4S traffic
 *
 * This test suite will run two types of traffic: Classic and Scalable traffic. It checks if Classic traffic has ECT1 codepoint set and
 * L4S traffic has ECT0 codepoint set 
 *
 */
class TcpDualQueueTest : public TcpGeneralTest
{
public:
  /**
   * \brief Constructor
   *
   * \param testcase test case number
   * \param desc Description about the type of traffic
   */
  TcpDualQueueTest (uint32_t testcase, const std::string &desc);

protected:
  virtual void Tx (const Ptr<const Packet> p, const TcpHeader&h, SocketWho who);
  virtual Ptr<TcpSocketMsgBase> CreateSenderSocket (Ptr<Node> node);
  virtual Ptr<TcpSocketMsgBase> CreateReceiverSocket (Ptr<Node> node);
  void ConfigureProperties ();

private:
  uint32_t m_senderSent;
  uint32_t m_testCase;
};

TcpDualQueueTest::TcpDualQueueTest (uint32_t testcase, const std::string &desc)
  : TcpGeneralTest (desc),
    m_senderSent (0),
    m_testCase (testcase)
{
}

void
TcpDualQueueTest::ConfigureProperties ()
{
  TcpGeneralTest::ConfigureProperties ();
  SetEcn (SENDER);
  SetEcn (RECEIVER);
}

void
TcpDualQueueTest::Tx (const Ptr<const Packet> p, const TcpHeader &h, SocketWho who)
{
  if (who == SENDER)
    {
      m_senderSent++;
      if (m_senderSent == 3)
        {
          SocketIpTosTag ipTosTag;
          p->PeekPacketTag (ipTosTag);
          if (m_testCase == 1)
            {
              NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x1, "IP TOS should have ECT1 set for L4S traffic");
            }
          else if (m_testCase == 2)
            {
              NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x2, "IP TOS should have ECT0 set for Classic traffic");
            }
        }
    }
}

Ptr<TcpSocketMsgBase>
TcpDualQueueTest::CreateSenderSocket (Ptr<Node> node)
{
  if (m_testCase == 1)
    {
      return TcpGeneralTest::CreateSocket (node, TcpSocketMsgBase::GetTypeId (), TcpDctcp::GetTypeId ());
    }
  else
    {
      return TcpGeneralTest::CreateSenderSocket (node);
    }
}

Ptr<TcpSocketMsgBase>
TcpDualQueueTest::CreateReceiverSocket (Ptr<Node> node)
{
  if (m_testCase == 1)
    {
      return TcpGeneralTest::CreateSocket (node, TcpSocketMsgBase::GetTypeId (), TcpDctcp::GetTypeId ());
    }
  else
    {
      return TcpGeneralTest::CreateReceiverSocket (node);
    }
}

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief TCP DualQueue TestSuite
 */
static class TcpDualQueueTestSuite : public TestSuite
{
public:
  TcpDualQueueTestSuite () : TestSuite ("tcp-dual-queue-test", UNIT)
  {
    AddTestCase (new TcpDualQueueTest (1, "L4S traffic"),
                 TestCase::QUICK);
    AddTestCase (new TcpDualQueueTest (2, "Classic traffic"),
                 TestCase::QUICK);
  }
} g_tcpDualQueueTestSuite;

} // namespace ns3
