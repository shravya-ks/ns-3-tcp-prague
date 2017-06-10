#ifndef TCP_DCTCP_H
#define TCP_DCTCP_H

#include "ns3/tcp-congestion-ops.h"

namespace ns3 {
class TcpDctcp : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * Create an unbound tcp socket.
   */
  TcpDctcp ();
  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpDctcp (const TcpDctcp& sock);
  virtual ~TcpDctcp (void);
  virtual std::string GetName () const;
  virtual void SetSocketBase (Ptr<TcpSocketBase> tsb);
  virtual Ptr<TcpCongestionOps> Fork ();
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time &rtt);

  virtual void CwndEvent (Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCaEvent_t event);


private:
  void CEState0to1 (Ptr<TcpSocketState> tcb);
  void CEState1to0 (Ptr<TcpSocketState> tcb);
  void UpdateAckReserved (Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCaEvent_t event);
  void Reset(Ptr<TcpSocketState> tcb);
  void SetDctcpAlpha (uint32_t alpha);
  Ptr<TcpSocketBase> m_tsb;
  uint32_t m_ackedBytesEcn;
  uint32_t m_ackedBytesTotal;
  SequenceNumber32 m_priorRcvNxt;
  bool m_priorRcvNxtFlag;
  uint32_t m_dctcpAlpha;
  SequenceNumber32 m_nextSeq;
  uint32_t m_ceState;
  bool m_delayedAckReserved;
  uint32_t m_lossCwnd;
  uint32_t m_dctcpShiftG;
  uint32_t m_dctcpClampAlphaOnLoss; 
};

}

#endif /*TCP_DCTCP_H*/

