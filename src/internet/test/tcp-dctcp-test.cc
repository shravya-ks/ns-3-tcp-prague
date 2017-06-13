//m_congestionControl->GetName () == "TcpDctcp"

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
#include "ns3/string.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpDctcpTestSuite");


class TcpDctcpEctTest : public TcpGeneralTest
{
public:
  /**
   * \brief Constructor
   *
   * \param desc Description about the test
   */
  TcpDctcpEctTest (const std::string &desc);

protected:
  virtual void Tx (const Ptr<const Packet> p, const TcpHeader&h, SocketWho who);
  void ConfigureProperties ();

private:
  uint32_t m_senderSent;
  uint32_t m_receiverSent;
};

void
TcpDctcpEctTest::Tx (const Ptr<const Packet> p, const TcpHeader &h, SocketWho who)
{
  if (who == SENDER)
    {
      m_senderSent++;
      SocketIpTosTag ipTosTag;
      p->PeekPacketTag (ipTosTag);
      if (m_senderSent == 1)
        {
          NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x2, "IP TOS should have ECT for SYN packet for DCTCP");
        }
      if (m_senderSent == 2)
        {
          NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x2, "IP TOS should have ECT for data packets for DCTCP");
        }
    }
  else
   {
     m_receiverSent++;
     SocketIpTosTag ipTosTag;
     p->PeekPacketTag (ipTosTag);
     if (m_receiverSent == 1)
       {
         NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x2, "IP TOS should have ECT for SYN+ACK packet for DCTCP");
       }
     if (m_receiverSent == 2)
       {
         NS_TEST_ASSERT_MSG_EQ ((ipTosTag.GetTos ()), 0x2, "IP TOS should have ECT for pure ACK packet for DCTCP");
       }
   }        
}

TcpDctcpEctTest::TcpDctcpEctTest (const std::string &desc)
  : TcpGeneralTest (desc),
    m_senderSent (0),
    m_receiverSent (0)
{
}

void
TcpDctcpEctTest::ConfigureProperties ()
{
  TcpGeneralTest::ConfigureProperties ();
  SetDctcp (SENDER);
  SetDctcp (RECEIVER);
}

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief DCTCP should be same as NewReno during slow start
 */
class TcpDctcpToNewReno : public TestCase
{
public:
  /**
   * \brief Constructor
   *
   * \param cWnd congestion window
   * \param segmentSize segment size
   * \param ssThresh slow start threshold
   * \param segmentsAcked segments acked
   * \param highTxMark high tx mark
   * \param lastAckedSeq last acked seq
   * \param rtt RTT
   * \param name Name of the test
   */
  TcpDctcpToNewReno (uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                      uint32_t segmentsAcked, SequenceNumber32 highTxMark,
                      SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name);

private:
  virtual void DoRun (void);
  /** \brief Execute the test
   */
  void ExecuteTest (void);

  uint32_t m_cWnd; //!< cWnd
  uint32_t m_segmentSize; //!< segment size
  uint32_t m_segmentsAcked; //!< segments acked
  uint32_t m_ssThresh; //!< ss thresh
  Time m_rtt; //!< rtt
  SequenceNumber32 m_highTxMark; //!< high tx mark
  SequenceNumber32 m_lastAckedSeq; //!< last acked seq
  Ptr<TcpSocketState> m_state; //!< state
};

TcpDctcpToNewReno::TcpDctcpToNewReno (uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                                        uint32_t segmentsAcked, SequenceNumber32 highTxMark,
                                        SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name)
  : TestCase (name),
    m_cWnd (cWnd),
    m_segmentSize (segmentSize),
    m_segmentsAcked (segmentsAcked),
    m_ssThresh (ssThresh),
    m_rtt (rtt),
    m_highTxMark (highTxMark),
    m_lastAckedSeq (lastAckedSeq)
{
}

void
TcpDctcpToNewReno::DoRun ()
{
  Simulator::Schedule (Seconds (0.0), &TcpDctcpToNewReno::ExecuteTest, this);
  Simulator::Run ();
  Simulator::Destroy ();
}

void
TcpDctcpToNewReno::ExecuteTest ()
{
  m_state = CreateObject <TcpSocketState> ();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_highTxMark = m_highTxMark;
  m_state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpSocketState> state = CreateObject <TcpSocketState> ();
  state->m_cWnd = m_cWnd;
  state->m_ssThresh = m_ssThresh;
  state->m_segmentSize = m_segmentSize;
  state->m_highTxMark = m_highTxMark;
  state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpDctcp> cong = CreateObject <TcpDctcp> ();
  cong->IncreaseWindow (m_state, m_segmentsAcked);

  Ptr<TcpNewReno> NewRenoCong = CreateObject <TcpNewReno> ();
  NewRenoCong->IncreaseWindow (state, m_segmentsAcked);

  NS_TEST_ASSERT_MSG_EQ (m_state->m_cWnd.Get (), state->m_cWnd.Get (),
                         "cWnd has not updated correctly");
}
/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief Test to validate cWnd decrement Dctcp
 */
class TcpDctcpDecrementTest : public TestCase
{
public:
  /**
   * \brief Constructor
   *
   * \param cWnd congestion window
   * \param segmentSize segment size
   * \param segmentsAcked segments acked
   * \param highTxMark high tx mark
   * \param lastAckedSeq last acked seq
   * \param rtt RTT
   * \param name Name of the test
   */
  TcpDctcpDecrementTest (uint32_t cWnd, uint32_t segmentSize, uint32_t segmentsAcked, SequenceNumber32 nextTxSequence,
                          SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name);

private:
  virtual void DoRun (void);
  /** \brief Execute the test
   */
  void ExecuteTest (void);

  uint32_t m_cWnd; //!< cWnd
  uint32_t m_segmentSize; //!< segment size
  uint32_t m_segmentsAcked; //!< segments acked
  Time m_rtt; //!< rtt
  SequenceNumber32 m_nextTxSequence; //!< next seq num to be sent
  SequenceNumber32 m_lastAckedSeq; //!< last acked seq
  Ptr<TcpSocketState> m_state; //!< state
};

TcpDctcpDecrementTest::TcpDctcpDecrementTest (uint32_t cWnd, uint32_t segmentSize, uint32_t segmentsAcked, SequenceNumber32 nextTxSequence,
                                                SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name)
  : TestCase (name),
    m_cWnd (cWnd),
    m_segmentSize (segmentSize),
    m_segmentsAcked (segmentsAcked),
    m_rtt (rtt),
    m_nextTxSequence (nextTxSequence),
    m_lastAckedSeq (lastAckedSeq)
{
}

void
TcpDctcpDecrementTest::DoRun ()
{
  Simulator::Schedule (Seconds (0.0), &TcpDctcpDecrementTest::ExecuteTest, this);
  Simulator::Run ();
  Simulator::Destroy ();
}

void
TcpDctcpDecrementTest::ExecuteTest (void)
{
  m_state = CreateObject <TcpSocketState> ();
  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_nextTxSequence = m_nextTxSequence;
  m_state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpDctcp> cong = CreateObject <TcpDctcp> ();
  m_state->m_ecnState = TcpSocketState::ECN_IDLE;
  cong->PktsAcked (m_state, m_segmentsAcked, m_rtt);
  cong->ReduceCwnd (m_state);
  NS_TEST_ASSERT_MSG_EQ (m_state->m_cWnd.Get (), m_cWnd,
                         "cWnd has updated correctly");

  m_state->m_ecnState = TcpSocketState::ECN_ECE_RCVD;
  cong->PktsAcked (m_state, m_segmentsAcked, m_rtt);
  cong->ReduceCwnd (m_state);

  uint32_t val = (uint32_t)(m_cWnd * (1 - 0.0625/2.0));
  NS_TEST_ASSERT_MSG_EQ (m_state->m_cWnd.Get (), val,
                         "cWnd has updated correctly");
  
}


/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief TCP Dctcp TestSuite
 */
class TcpDctcpTestSuite : public TestSuite
{
public:
  TcpDctcpTestSuite () : TestSuite ("tcp-dctcp-test", UNIT)
  {
    AddTestCase (new TcpDctcpEctTest ("ECT Test : Check if ECT is set on Syn, Syn+ Ack, Ack and Data packets"),
                 TestCase::QUICK);
    AddTestCase (new TcpDctcpToNewReno (2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32 (4753), SequenceNumber32 (3216), MilliSeconds (100), "DCTCP falls to New Reno for slowstart"), TestCase::QUICK);
    AddTestCase (new TcpDctcpDecrementTest (4 * 1446, 1446, 2, SequenceNumber32 (3216), SequenceNumber32 (4753), MilliSeconds (100), "DCTCP decrement test"), TestCase::QUICK);
   // AddTestCase (new TcpLedbatIncrementTest (2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32 (4753), SequenceNumber32 (3216), MilliSeconds (100), "LEDBAT increment test"), TestCase::QUICK);
    //AddTestCase (new TcpLedbatDecrementTest (2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32 (4753), SequenceNumber32 (3216), MilliSeconds (100), "LEDBAT decrement test"), TestCase::QUICK);
  }
};

static TcpDctcpTestSuite g_tcpdctcpTest; //!< static var for test initialization
