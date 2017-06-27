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

#ifndef POINT_TO_POINT_BCUBE_HELPER_H
#define POINT_TO_POINT_BCUBE_HELPER_H

#include <vector>

#include "internet-stack-helper.h"
#include "point-to-point-helper.h"
#include "ipv4-address-helper.h"
#include "ipv6-address-helper.h"
#include "ipv4-interface-container.h"
#include "ipv6-interface-container.h"
#include "net-device-container.h"

namespace ns3 {

/**
 * \ingroup point-to-point-layout
 *
 * \brief A helper to make it easier to create a BCube topology
 * with p2p links
 */
class PointToPointBCubeHelper
{
public:
  /**
   * Create a PointToPointBCubeHelper in order to easily create
   * BCube topologies using p2p links
   *
   * \param nLevels total number of levels in BCube
   *
   * \param nServers total number of servers in one BCube
   *
   * \param pointToPoint the PointToPointHelper which is used
   *                     to connect all of the nodes together
   *                     in the BCube
   */
  PointToPointBCubeHelper (uint32_t nLevels,
                           uint32_t nServers,
                           PointToPointHelper pointToPoint);

  ~PointToPointBCubeHelper ();

  /**
   * \param row the row address of the switch desired
   *
   * \param col the column address of the switch desired
   *
   * \returns a pointer to the switch specified by the
   *          (row, col) address
   */
  Ptr<Node> GetSwitchNode (uint32_t row, uint32_t col) const;

  /**
   * \param col the column address of the host desired
   *
   * \returns a pointer to the host specified by the
   *          column address
   */
  Ptr<Node> GetHostNode (uint32_t col) const;

  /**
   * This returns an Ipv4 address at the switch specified by
   * the (row, col) address.  Technically, a switch will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the BCube, the address returned
   * is the lowest index host connected to this switch.
   *
   * \param row the row address of the switch desired
   *
   * \param col the column address of the switch desired
   *
   * \returns Ipv4Address of one of the interfaces of the switch
   *          specified by the (row, col) address
   */
  Ipv4Address GetSwitchIpv4Address (uint32_t row, uint32_t col) const;

  /**
   * This returns an Ipv6 address at the switch specified by
   * the (row, col) address.  Technically, a switch will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the BCube, the address returned
   * is the lowest index host connected to this switch.
   *
   * \param row the row address of the switch desired
   *
   * \param col the column address of the switch desired
   *
   * \returns Ipv6Address of one of the interfaces of the switch
   *          specified by the (row, col) address
   */
  Ipv6Address GetSwitchIpv6Address (uint32_t row, uint32_t col) const;

  /**
   * This returns an Ipv4 address at the host specified by
   * the col address.  Technically, a host will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the BCube, the address returned
   * is the switch at lowest level connected to this host.
   *
   * \param col the column address of the host desired
   *
   * \returns Ipv4Address of one of the interfaces of the host
   *          specified by the col address
   */
  Ipv4Address GetHostIpv4Address (uint32_t col) const;

  /**
   * This returns an Ipv6 address at the host specified by
   * the col address.  Technically, a host will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the BCube, the address returned
   * is the switch at lowest level connected to this host.
   *
   * \param col the column address of the host desired
   *
   * \returns Ipv6Address of one of the interfaces of the host
   *          specified by the col address
   */
  Ipv6Address GetHostIpv6Address (uint32_t col) const;

  /**
   * \param stack an InternetStackHelper which is used to install
   *              on every node in the BCube
   */
  void InstallStack (InternetStackHelper stack);

  /**
   * Assigns Ipv4 addresses to all the interfaces of switch
   *
   * \param network an IPv4 address representing the network portion
   *                of the IPv4 Address
   *
   * \param mask the mask length
   */
  void AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask);

  /**
   * Assigns Ipv6 addresses to all the interfaces of the switch
   *
   * \param network an IPv6 address representing the network portion
   *                of the IPv6 Address
   *
   * \param prefix the prefix length
   */
  void AssignIpv6Addresses (Ipv6Address network, Ipv6Prefix prefix);

  /**
   * Sets up the node canvas locations for every node in the BCube.
   * This is needed for use with the animation interface
   *
   * \param ulx upper left x value
   * \param uly upper left y value
   * \param lrx lower right x value
   * \param lry lower right y value
   */
  void BoundingBox (double ulx, double uly, double lrx, double lry);

private:
  uint32_t m_numLevels;                                         //!< number of levels
  uint32_t m_numServers;                                        //!< number of servers
  std::vector<NetDeviceContainer> m_levelSwitchDevices;         //!< Net Device container for hosts and switches
  std::vector<Ipv4InterfaceContainer> m_switchInterfaces;       //!< IPv4 interfaces of switch
  Ipv4InterfaceContainer m_hostInterfaces;                      //!< IPv4 interfaces of host
  std::vector<Ipv6InterfaceContainer> m_switchInterfaces6;      //!< IPv6 interfaces of switch
  Ipv6InterfaceContainer m_hostInterfaces6;                     //!< IPv6 interfaces of host
  NodeContainer m_switches;                                     //!< all the switches in the bcube
  NodeContainer m_hosts;                                        //!< all the hosts in the bcube
};

} // namespace ns3

#endif /* POINT_TO_POINT_BCUBE_HELPER_H */
