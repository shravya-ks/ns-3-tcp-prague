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
 * Authors: Shravya K.S. <shravya.ks0@gmail.com>
 *
 */

#include "ns3/point-to-point-fattree.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv6-address-generator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointFatTreeHelper");

PointToPointFatTreeHelper::PointToPointFatTreeHelper (uint32_t numPods,
                                                      PointToPointHelper p2pHelper)
  : m_numPods (numPods)
{
  // Bounds check
  if (numPods < 0)
    {
      NS_FATAL_ERROR ("Need more pods for FatTree.");
    }
  if (numPods % 2 != 0)
    {
      NS_FATAL_ERROR ("Number of pods should be even in FatTree.");
    }

  uint32_t numEdgeSwitches = numPods / 2;
  uint32_t numAggregateSwitches = numPods / 2;            // number of aggregation switch in a pod
  uint32_t numGroups = numPods / 2;                       // number of group of core switches
  uint32_t numCoreSwitches = numPods / 2;                 // number of core switch in a group
  uint32_t numServers = numPods * numPods * numPods / 4;      // number of hosts in the entire network
  m_edgeSwitchDevices.resize (numPods * numEdgeSwitches);
  m_aggregateSwitchDevices.resize (numPods * numAggregateSwitches);
  m_coreSwitchDevices.resize (numGroups * numCoreSwitches);

  m_servers.Create (numServers);
  m_edgeSwitches.Create (numEdgeSwitches * numPods);
  m_aggregateSwitches.Create (numAggregateSwitches * numPods);
  m_coreSwitches.Create (numCoreSwitches * numGroups);

  InternetStackHelper stack;

  //Connect servers to edge switches
  uint32_t hostId = 0;
  for (uint32_t i = 0; i < numPods * numPods / 2; i++)
    {
      for (uint32_t j = 0; j < numEdgeSwitches; j++)
        {
          NetDeviceContainer nd = p2pHelper.Install (m_servers.Get (hostId), m_edgeSwitches.Get (i));
          m_edgeSwitchDevices[i].Add (nd.Get (0));
          m_edgeSwitchDevices[i].Add (nd.Get (1));
          hostId += 1;
        }
    }

  //Connect edge switches to aggregate switches
  for (uint32_t i = 0; i < numPods; i++)
    {
      for (uint32_t j = 0; j < numAggregateSwitches; j++)
        {
          for (uint32_t k = 0; k < numEdgeSwitches; k++)
            {
              NetDeviceContainer nd = p2pHelper.Install (m_edgeSwitches.Get (i * numEdgeSwitches + k),
                                                         m_aggregateSwitches.Get (i * numAggregateSwitches + j));
              m_aggregateSwitchDevices[i * numAggregateSwitches + j].Add (nd.Get (0));
              m_aggregateSwitchDevices[i * numAggregateSwitches + j].Add (nd.Get (1));
            }
        }
    }

  //Connect aggregate switches to core switches
  for (uint32_t i = 0; i < numGroups; i++)
    {
      for (uint32_t j = 0; j < numCoreSwitches; j++)
        {
          for (uint32_t k = 0; k < numPods; k++)
            {
              NetDeviceContainer nd = p2pHelper.Install (m_aggregateSwitches.Get (k * numAggregateSwitches + i),
                                                         m_coreSwitches.Get (i * numCoreSwitches + j));
              m_coreSwitchDevices[i * numCoreSwitches + j].Add (nd.Get (0));
              m_coreSwitchDevices[i * numCoreSwitches + j].Add (nd.Get (1));
            }
        }
    }

}

PointToPointFatTreeHelper::~PointToPointFatTreeHelper ()
{
}

void
PointToPointFatTreeHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install (m_servers);
  stack.Install (m_edgeSwitches);
  stack.Install (m_aggregateSwitches);
  stack.Install (m_coreSwitches);
}

void
PointToPointFatTreeHelper::BoundingBox (double ulx, double uly,
                                        double lrx, double lry)
{
  NS_LOG_FUNCTION (this << ulx << uly << lrx << lry);
  double xDist;
  double yDist;
  if (lrx > ulx)
    {
      xDist = lrx - ulx;
    }
  else
    {
      xDist = ulx - lrx;
    }
  if (lry > uly)
    {
      yDist = lry - uly;
    }
  else
    {
      yDist = uly - lry;
    }

  uint32_t numServers = m_numPods * m_numPods * m_numPods / 4;
  uint32_t numSwitches = m_numPods * m_numPods / 2;

  double xServerAdder = xDist / numServers;
  double xEdgeSwitchAdder = xDist / numSwitches;
  double xAggregateSwitchAdder =  xDist / numSwitches;
  double xCoreSwitchAdder = xDist / (numSwitches / 2);
  double  yAdder = yDist / 4;  // 3 layers of switches and 1 layer of servers

  //Allot servers
  double xLoc;
  double yLoc = yDist / 2;
  for (uint32_t i = 0; i < numServers; ++i)
    {
      //xLoc = xDist / 2;
      Ptr<Node> node = m_servers.Get (i);
      Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
      if (loc == 0)
        {
          loc = CreateObject<ConstantPositionMobilityModel> ();
          node->AggregateObject (loc);
        }
      Vector locVec (xLoc, yLoc, 0);
      loc->SetPosition (locVec);
      if (i % 2 == 0)
        {
          xLoc += 3 * xServerAdder;
        }
      else
        {
          xLoc += 1.1 * xServerAdder;
        }
    }

  yLoc -= yAdder;

  //Allot edge switches
  xLoc = xEdgeSwitchAdder;
  for (uint32_t i = 0; i < numSwitches; ++i)
    {
      Ptr<Node> node = m_edgeSwitches.Get (i);
      Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
      if (loc == 0)
        {
          loc = CreateObject<ConstantPositionMobilityModel> ();
          node->AggregateObject (loc);
        }
      Vector locVec (xLoc, yLoc, 0);
      loc->SetPosition (locVec);
      xLoc += 2 * xEdgeSwitchAdder;
    }

  yLoc -= yAdder;

  //Allot aggregate switches
  xLoc = xAggregateSwitchAdder;
  for (uint32_t i = 0; i < numSwitches; ++i)
    {
      Ptr<Node> node = m_aggregateSwitches.Get (i);
      Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
      if (loc == 0)
        {
          loc = CreateObject<ConstantPositionMobilityModel> ();
          node->AggregateObject (loc);
        }
      Vector locVec (xLoc, yLoc, 0);
      loc->SetPosition (locVec);
      xLoc += 2 * xAggregateSwitchAdder;
    }

  yLoc -= yAdder;

  //Allot aggregate switches
  xLoc = xCoreSwitchAdder;
  for (uint32_t i = 0; i < numSwitches / 2; ++i)
    {
      Ptr<Node> node = m_coreSwitches.Get (i);
      Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
      if (loc == 0)
        {
          loc = CreateObject<ConstantPositionMobilityModel> ();
          node->AggregateObject (loc);
        }
      Vector locVec (xLoc, yLoc, 0);
      loc->SetPosition (locVec);
      xLoc += 2 * xCoreSwitchAdder;
    }
}


void
PointToPointFatTreeHelper::AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask)
{
  NS_LOG_FUNCTION (this << network << mask);
  Ipv4AddressGenerator::Init (network, mask);
  Ipv4Address v4network;
  Ipv4AddressHelper addrHelper;

  for (uint32_t i = 0; i < m_edgeSwitchDevices.size (); ++i)
    {
      for (uint32_t j = 0; j < m_edgeSwitchDevices[i].GetN (); j += 2)
        {
          v4network = Ipv4AddressGenerator::NextNetwork (mask);
          addrHelper.SetBase (v4network, mask);
          Ipv4InterfaceContainer ic = addrHelper.Assign (m_edgeSwitchDevices[i].Get (j));
          m_serverInterfaces.Add (ic);
          ic = addrHelper.Assign (m_edgeSwitchDevices[i].Get (j + 1));
          m_edgeSwitchInterfaces.Add (ic);
        }
    }

  for (uint32_t i = 0; i < m_aggregateSwitchDevices.size (); ++i)
    {
      v4network = Ipv4AddressGenerator::NextNetwork (mask);
      addrHelper.SetBase (v4network, mask);
      for (uint32_t j = 0; j < m_aggregateSwitchDevices[i].GetN (); j += 2)
        {
          Ipv4InterfaceContainer ic = addrHelper.Assign (m_aggregateSwitchDevices[i].Get (j));
          m_edgeSwitchInterfaces.Add (ic);
          ic = addrHelper.Assign (m_aggregateSwitchDevices[i].Get (j + 1));
          m_aggregateSwitchInterfaces.Add (ic);
        }
    }

  for (uint32_t i = 0; i < m_coreSwitchDevices.size (); ++i)
    {
      v4network = Ipv4AddressGenerator::NextNetwork (mask);
      addrHelper.SetBase (v4network, mask);
      for (uint32_t j = 0; j < m_coreSwitchDevices[i].GetN (); j += 2)
        {
          Ipv4InterfaceContainer ic = addrHelper.Assign (m_coreSwitchDevices[i].Get (j));
          m_aggregateSwitchInterfaces.Add (ic);
          ic = addrHelper.Assign (m_coreSwitchDevices[i].Get (j + 1));
          m_coreSwitchInterfaces.Add (ic);
        }
    }
}


void
PointToPointFatTreeHelper::AssignIpv6Addresses (Ipv6Address addrBase, Ipv6Prefix prefix)
{
  NS_LOG_FUNCTION (this << addrBase << prefix);
  Ipv6AddressGenerator::Init (addrBase, prefix);
  Ipv6Address v6network;
  Ipv6AddressHelper addrHelper;

  for (uint32_t i = 0; i < m_edgeSwitchDevices.size (); ++i)
    {
      v6network = Ipv6AddressGenerator::NextNetwork (prefix);
      addrHelper.SetBase (v6network, prefix);
      for (uint32_t j = 0; j < m_edgeSwitchDevices[i].GetN (); j += 2)
        {
          Ipv6InterfaceContainer ic = addrHelper.Assign (m_edgeSwitchDevices[i].Get (j));
          m_serverInterfaces6.Add (ic);
          ic = addrHelper.Assign (m_edgeSwitchDevices[i].Get (j + 1));
          m_edgeSwitchInterfaces6.Add (ic);
        }
    }

  for (uint32_t i = 0; i < m_aggregateSwitchDevices.size (); ++i)
    {
      v6network = Ipv6AddressGenerator::NextNetwork (prefix);
      addrHelper.SetBase (v6network, prefix);
      for (uint32_t j = 0; j < m_aggregateSwitchDevices[i].GetN (); j += 2)
        {
          Ipv6InterfaceContainer ic = addrHelper.Assign (m_aggregateSwitchDevices[i].Get (j));
          m_edgeSwitchInterfaces6.Add (ic);
          ic = addrHelper.Assign (m_aggregateSwitchDevices[i].Get (j + 1));
          m_aggregateSwitchInterfaces6.Add (ic);
        }
    }

  for (uint32_t i = 0; i < m_coreSwitchDevices.size (); ++i)
    {
      v6network = Ipv6AddressGenerator::NextNetwork (prefix);
      addrHelper.SetBase (v6network, prefix);
      for (uint32_t j = 0; j < m_coreSwitchDevices[i].GetN (); j += 2)
        {
          Ipv6InterfaceContainer ic = addrHelper.Assign (m_coreSwitchDevices[i].Get (j));
          m_aggregateSwitchInterfaces6.Add (ic);
          ic = addrHelper.Assign (m_coreSwitchDevices[i].Get (j + 1));
          m_coreSwitchInterfaces6.Add (ic);
        }
    }

}

Ipv4Address
PointToPointFatTreeHelper::GetServerIpv4Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_serverInterfaces.GetAddress (i);
}

Ipv4Address
PointToPointFatTreeHelper::GetEdgeSwitchIpv4Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_edgeSwitchInterfaces.GetAddress (i);
}

Ipv4Address
PointToPointFatTreeHelper::GetAggregateSwitchIpv4Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_aggregateSwitchInterfaces.GetAddress (i);
}

Ipv4Address
PointToPointFatTreeHelper::GetCoreSwitchIpv4Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_coreSwitchInterfaces.GetAddress (i);
}

Ipv6Address
PointToPointFatTreeHelper::GetServerIpv6Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_serverInterfaces6.GetAddress (i, 1);
}

Ipv6Address
PointToPointFatTreeHelper::GetEdgeSwitchIpv6Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_edgeSwitchInterfaces6.GetAddress (i, 1);
}

Ipv6Address
PointToPointFatTreeHelper::GetAggregateSwitchIpv6Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_aggregateSwitchInterfaces6.GetAddress (i, 1);
}

Ipv6Address
PointToPointFatTreeHelper::GetCoreSwitchIpv6Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_coreSwitchInterfaces6.GetAddress (i, 1);
}

Ptr<Node>
PointToPointFatTreeHelper::GetServerNode (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_servers.Get (i);
}

Ptr<Node>
PointToPointFatTreeHelper::GetEdgeSwitchNode (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_edgeSwitches.Get (i);
}

Ptr<Node>
PointToPointFatTreeHelper::GetAggregateSwitchNode (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_aggregateSwitches.Get (i);
}

Ptr<Node>
PointToPointFatTreeHelper::GetCoreSwitchNode (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_coreSwitches.Get (i);
}

} // namespace ns3
