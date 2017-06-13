#include "tcp-dctcp.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include "math.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/sequence-number.h"
#include "ns3/double.h"
#include "ns3/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpDctcp");

NS_OBJECT_ENSURE_REGISTERED (TcpDctcp);

TypeId TcpDctcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpDctcp")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpDctcp> ()
    .SetGroupName ("Internet")
    .AddAttribute ("DctcpShiftG",
                   "Parameter G for updating dctcp_alpha",
                   DoubleValue (0.0625),
                   MakeDoubleAccessor (&TcpDctcp::m_dctcpShiftG),
                   MakeDoubleChecker<double> (0))
    .AddAttribute ("DctcpAlphaOnInit",
                   "Parameter for initial alpha value",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&TcpDctcp::SetDctcpAlpha),
                   MakeDoubleChecker<double> (0))
  ;
  return tid;
}

std::string TcpDctcp::GetName () const
{
  return "TcpDctcp";
}

TcpDctcp::TcpDctcp ()
  : TcpNewReno (),
    m_tsb(0)
{
  NS_LOG_FUNCTION (this);
  m_delayedAckReserved = false;
  m_ceState = 0;
  m_ackedBytesEcn = 0;
  m_ackedBytesTotal = 0;
  m_priorRcvNxtFlag = false;
  m_nextSeqFlag = false;
}

TcpDctcp::TcpDctcp (const TcpDctcp& sock)
  : TcpNewReno (sock),
    m_tsb (sock.m_tsb)
{
  NS_LOG_FUNCTION (this);
    m_delayedAckReserved = (sock.m_delayedAckReserved);
    m_ceState = (sock.m_ceState);
}

TcpDctcp::~TcpDctcp (void)
{
  NS_LOG_FUNCTION (this);
}

void
TcpDctcp::SetSocketBase (Ptr<TcpSocketBase> tsb)
{
  m_tsb = tsb;
}
Ptr<TcpCongestionOps> TcpDctcp::Fork (void)
{
  NS_LOG_FUNCTION (this);
  return CopyObject<TcpDctcp> (this);
}

void 
TcpDctcp::ReduceCwnd (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  uint32_t val = (int)((1 - m_dctcpAlpha/2.0) * tcb->m_cWnd);
  tcb->m_cWnd = std::max(val,tcb->m_segmentSize);
}

void 
TcpDctcp::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time &rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);
  m_ackedBytesTotal += segmentsAcked * tcb->m_segmentSize;
  if (tcb->m_ecnState == TcpSocketState::ECN_ECE_RCVD)
    {
      m_ackedBytesEcn += segmentsAcked * tcb->m_segmentSize;
    }
  if (m_nextSeqFlag == false)
    {
      m_nextSeq = tcb->m_nextTxSequence;
      m_nextSeqFlag = true;
    }
  if (tcb->m_lastAckedSeq >= m_nextSeq)
    {
      double bytesEcn;  
      if(m_ackedBytesTotal >  0)
        {
          bytesEcn = (double)m_ackedBytesEcn/m_ackedBytesTotal;
        }
      else
        {
          bytesEcn = 0.0;
        }
      m_dctcpAlpha = (1.0 - m_dctcpShiftG) * m_dctcpAlpha + m_dctcpShiftG * bytesEcn;
      Reset(tcb);
    }
}

void
TcpDctcp::SetDctcpAlpha (double alpha)
{
  NS_LOG_FUNCTION (this << alpha);
  m_dctcpAlpha = alpha;
}

void 
TcpDctcp::Reset(Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  m_nextSeq = tcb->m_nextTxSequence;
  m_ackedBytesEcn = 0;
  m_ackedBytesTotal = 0;
}

void 
TcpDctcp::CEState0to1 (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  if (!m_ceState && m_delayedAckReserved && m_priorRcvNxtFlag) 
    {
      SequenceNumber32 tmpRcvNxt;
      /* Save current NextRxSequence. */
      tmpRcvNxt = m_tsb->m_rxBuffer->NextRxSequence ();
   
      /* Generate previous ack without ECE */
      m_tsb->m_rxBuffer->SetNextRxSequence (m_priorRcvNxt);
      m_tsb->SendEmptyPacket (TcpHeader::ACK);

      /* Recover current rcv_nxt. */
      m_tsb->m_rxBuffer->SetNextRxSequence (tmpRcvNxt);
   }

  if(m_priorRcvNxtFlag == false)
    {
      m_priorRcvNxtFlag = true;
    }
   m_priorRcvNxt = m_tsb->m_rxBuffer->NextRxSequence ();
   m_ceState = 1;
   tcb->m_ecnState = TcpSocketState::ECN_CE_RCVD;  
}

void 
TcpDctcp::CEState1to0 (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  NS_LOG_DEBUG("here");
  if (m_ceState && m_delayedAckReserved && m_priorRcvNxtFlag) 
    {
      SequenceNumber32 tmpRcvNxt;
      /* Save current NextRxSequence. */
      tmpRcvNxt = m_tsb->m_rxBuffer->NextRxSequence ();
     
      /* Generate previous ack with ECE */
      m_tsb->m_rxBuffer->SetNextRxSequence (m_priorRcvNxt);
      m_tsb->SendEmptyPacket (TcpHeader::ACK | TcpHeader::ECE);
      NS_LOG_DEBUG("ECE flags with Send Empty Packet called");

      /* Recover current rcv_nxt. */
      m_tsb->m_rxBuffer->SetNextRxSequence (tmpRcvNxt);
   }
  
   if(m_priorRcvNxtFlag == false)
    {
      m_priorRcvNxtFlag = true;
    }
   m_priorRcvNxt = m_tsb->m_rxBuffer->NextRxSequence ();
   m_ceState = 0;
   tcb->m_ecnState = TcpSocketState::ECN_IDLE; 
}

void 
TcpDctcp::UpdateAckReserved (Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCaEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  switch (event) 
    {
      case TcpSocketState::CA_EVENT_DELAYED_ACK:
		if (!m_delayedAckReserved)
                  m_delayedAckReserved = true;
		break;
	case TcpSocketState::CA_EVENT_NON_DELAYED_ACK:
		if (m_delayedAckReserved)
                  m_delayedAckReserved = false;
		break;
	default:
		/* Don't care for the rest. */
		break;
	}
}

void 
TcpDctcp::CwndEvent(Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCaEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  switch (event) 
    {
      case TcpSocketState::CA_EVENT_ECN_IS_CE:
		CEState0to1(tcb);
		break;
	case TcpSocketState::CA_EVENT_ECN_NO_CE:
		CEState1to0(tcb);
		break;
	case TcpSocketState::CA_EVENT_DELAYED_ACK:
	case TcpSocketState::CA_EVENT_NON_DELAYED_ACK:
		UpdateAckReserved(tcb, event);
		break;
	default:
		/* Don't care for the rest. */
		break;
	}

}
}
