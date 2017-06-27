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

#include "ns3/point-to-point-bcube.h"
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

NS_LOG_COMPONENT_DEFINE ("PointToPointBCubeHelper");

PointToPointBCubeHelper::PointToPointBCubeHelper (uint32_t nLevels, 
                                                uint32_t nServers, 
                                                PointToPointHelper p2pHelper)
 : m_numLevels (nLevels), m_numServers (nServers)
{
  // Bounds check
  if (nLevels < 0 || nServers < 1)
    {
      NS_FATAL_ERROR ("Need more nodes for BCube.");
    }
  m_levelSwitchDevices.resize(nLevels+1);
  m_switchInterfaces.resize(nLevels+1);
  uint32_t numLevelSwitches = pow (nServers,nLevels);

  //Number of hosts = pow(n,k+1)	
  m_hosts.Create (nServers*numLevelSwitches);

  //Number of switches = (k+1)*pow(n,k)
  m_switches.Create ((nLevels+1)*numLevelSwitches);

  InternetStackHelper stack;
  uint32_t switchColId;

  for (uint32_t level =0; level < nLevels+1; level++)
    {
      switchColId =0;
      uint32_t val1 = pow(nServers,level);
      uint32_t val2 = val1 * nServers;

      for(uint32_t j = 0; j < numLevelSwitches; j++)
      {
        uint32_t hostIndex = j%val1 + j/val1 * val2;
        for (uint32_t k = hostIndex; k < (hostIndex + val2); k+=val1)
        { 
          NetDeviceContainer nd = p2pHelper.Install (m_hosts.Get (k), m_switches.Get (level*numLevelSwitches + switchColId));
          //m_hostDevices.Add (nd.Get (0));
          //m_switchDevices.Add (nd.Get (1));
          m_levelSwitchDevices[level].Add (nd.Get (0));
          m_levelSwitchDevices[level].Add (nd.Get (1));
        }
        switchColId +=1;
      }
    }
 
}

PointToPointBCubeHelper::~PointToPointBCubeHelper ()
{
}

void
PointToPointBCubeHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install (m_hosts);
  stack.Install (m_switches);
}

void
PointToPointBCubeHelper::BoundingBox (double ulx, double uly,
                                     double lrx, double lry)
{
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
  
  uint32_t val = pow (m_numServers,m_numLevels);
  uint32_t numHosts = val * m_numServers;
  double xHostAdder = xDist/numHosts;

 // uint32_t numSwitches = (m_numLevels + 1) * val;
  double xSwitchAdder = m_numServers * xHostAdder;
  double  yAdder = yDist/(m_numLevels + 2);  // (m_numLevels + 1) layers of switches and 1 layer of hosts

  //Allot hosts
  double xLoc;
  double yLoc = yDist / 2; 
  for (uint32_t i = 0; i < numHosts; ++i)
  {
     //xLoc = xDist / 4;
     Ptr<Node> node = m_hosts.Get(i);
     Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
     if (loc ==0)
       {
         loc = CreateObject<ConstantPositionMobilityModel> ();
         node->AggregateObject (loc);
       }
     Vector locVec (xLoc, yLoc, 0);
     loc->SetPosition (locVec);
     xLoc += 2*xHostAdder;
  }

  yLoc -= yAdder;
  
  //Allot Switches
  for (uint32_t i = 0; i < m_numLevels + 1; ++i)
    {
      if(m_numServers % 2 == 0)
        xLoc = xSwitchAdder/2 + xHostAdder ;
      else
        xLoc = xSwitchAdder/2 + xHostAdder/2;
      for (uint32_t j = 0; j < val; ++j)
        {
          Ptr<Node> node = m_switches.Get(i * val + j);
          Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
          if (loc ==0)
            {
              loc = CreateObject<ConstantPositionMobilityModel> ();
              node->AggregateObject (loc);
            }
          Vector locVec (xLoc, yLoc, 0);
          loc->SetPosition (locVec);

          xLoc += 2*xSwitchAdder;
        }
      yLoc -= yAdder;
    }
}

void
PointToPointBCubeHelper::AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask)
{
  Ipv4AddressGenerator::Init(network, mask);
  Ipv4Address v4network;
  Ipv4AddressHelper addrHelper;

  for(uint32_t i = 0; i < m_levelSwitchDevices.size (); ++i)
    {
          v4network = Ipv4AddressGenerator::NextNetwork (mask);
          addrHelper.SetBase(v4network, mask);
          for(uint32_t j = 0; j < m_levelSwitchDevices[i].GetN (); j += 2)
            {
              NS_LOG_DEBUG("here");
              Ipv4InterfaceContainer ic = addrHelper.Assign (m_levelSwitchDevices[i].Get(j));
              m_hostInterfaces.Add (ic);
              ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j+1));
              m_switchInterfaces[i].Add (ic);
            }
    }
}


void
PointToPointBCubeHelper::AssignIpv6Addresses(Ipv6Address addrBase, Ipv6Prefix prefix)
{
  Ipv6AddressGenerator::Init(addrBase, prefix);
  Ipv6Address v6network;
  Ipv6AddressHelper addrHelper;

  for(uint32_t i = 0; i < m_levelSwitchDevices.size (); ++i)
    {
          v6network = Ipv6AddressGenerator::NextNetwork (prefix);
          addrHelper.SetBase(v6network, prefix);
          for(uint32_t j = 0; j < m_levelSwitchDevices[i].GetN (); j += 2)
            {
              Ipv6InterfaceContainer ic = addrHelper.Assign (m_levelSwitchDevices[i].Get(j));
              m_hostInterfaces6.Add (ic);
              ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j+1));
              m_switchInterfaces6[i].Add (ic);
            }
    }
}

Ipv4Address
PointToPointBCubeHelper::GetHostIpv4Address (uint32_t i) const
{
  return m_hostInterfaces.GetAddress (i);
}

Ipv4Address
PointToPointBCubeHelper::GetSwitchIpv4Address (uint32_t i, uint32_t j) const
{
  return m_switchInterfaces[i].GetAddress (j);
}

Ipv6Address
PointToPointBCubeHelper::GetHostIpv6Address (uint32_t i) const
{
  return m_hostInterfaces6.GetAddress (i, 1);
}

Ipv6Address
PointToPointBCubeHelper::GetSwitchIpv6Address (uint32_t i, uint32_t j) const
{
  return m_switchInterfaces6[i].GetAddress (j, 1);
}

 
/*
Ptr<Node> 
PointToPointGridHelper::GetNode (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 || 
      col > m_nodes.at (row).GetN () - 1) 
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPointGridHelper::GetNode.");
    }

  return (m_nodes.at (row)).Get (col);
}

Ipv4Address
PointToPointGridHelper::GetIpv4Address (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 || 
      col > m_nodes.at (row).GetN () - 1) 
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPointGridHelper::GetIpv4Address.");
    }

  // Right now this just gets one of the addresses of the
  // specified node.  The exact device can't be specified.
  // If you picture the grid, the address returned is the 
  // address of the left (row) device of all nodes, with 
  // the exception of the left-most nodes in the grid; 
  // in which case the right (row) device address is 
  // returned
  if (col == 0)
    {
      return (m_rowInterfaces.at (row)).GetAddress (0);
    }
  else
    {
      return (m_rowInterfaces.at (row)).GetAddress ((2*col)-1);
    }
}

Ipv6Address
PointToPointGridHelper::GetIpv6Address (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 ||
      col > m_nodes.at (row).GetN () - 1)
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPointGridHelper::GetIpv4Address.");
    }

  // Right now this just gets one of the addresses of the
  // specified node.  The exact device can't be specified.
  // If you picture the grid, the address returned is the
  // address of the left (row) device of all nodes, with
  // the exception of the left-most nodes in the grid;
  // in which case the right (row) device address is
  // returned
  if (col == 0)
    {
      return (m_rowInterfaces6.at (row)).GetAddress (0, 1);
    }
  else
    {
      return (m_rowInterfaces6.at (row)).GetAddress ((2*col)-1, 1);
    }
}*/

} // namespace ns3
