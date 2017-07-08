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

#ifndef POINT_TO_POINT_FATTREE_HELPER_H
#define POINT_TO_POINT_FATTREE_HELPER_H

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
 * \brief A helper to make it easier to create a Fat-Tree topology
 * with p2p links
 */
class PointToPointFatTreeHelper
{
public:
  /**
   * Create a PointToPointFatTreeHelper in order to easily create
   * FatTree topologies using p2p links
   *
   * \param numPods total number of Pods in FatTree
   *
   * \param pointToPoint the PointToPointHelper which is used
   *                     to connect all of the nodes together
   *                     in the FatTree
   */
  PointToPointFatTreeHelper (uint32_t numPods,
                             PointToPointHelper pointToPoint);

  ~PointToPointFatTreeHelper ();

  /**
   * \param col the column address of the edge switch desired
   *
   * \returns a pointer to the edge switch specified by the
   *          column address
   */
  Ptr<Node> GetEdgeSwitchNode (uint32_t col) const;

  /**
   * \param col the column address of the aggregate switch desired
   *
   * \returns a pointer to the aggregate switch specified by the
   *          column address
   */
  Ptr<Node> GetAggregateSwitchNode (uint32_t col) const;

  /**
   * \param col the column address of the core switch desired
   *
   * \returns a pointer to the core switch specified by the
   *          column address
   */
  Ptr<Node> GetCoreSwitchNode (uint32_t col) const;

  /**
   * \param col the column address of the server desired
   *
   * \returns a pointer to the server specified by the
   *          column address
   */
  Ptr<Node> GetServerNode (uint32_t col) const;

  /**
   * This returns an Ipv4 address at the edge switch specified by
   * column address.  Technically, an edge switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the edge switch desired
   *
   * \returns Ipv4Address of one of the interfaces of the edge switch
   *          column address
   */
  Ipv4Address GetEdgeSwitchIpv4Address (uint32_t col) const;

  /**
   * This returns an Ipv4 address at the aggregate switch specified by
   * column address.  Technically, an aggregate switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the aggregate switch desired
   *
   * \returns Ipv4Address of one of the interfaces of the aggregate switch
   *          column address
   */
  Ipv4Address GetAggregateSwitchIpv4Address (uint32_t col) const;

  /**
   * This returns an Ipv4 address at the core switch specified by
   * column address.  Technically, a core switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the core switch desired
   *
   * \returns Ipv4Address of one of the interfaces of the core switch
   *          column address
   */
  Ipv4Address GetCoreSwitchIpv4Address (uint32_t col) const;

  /**
   * This returns an Ipv6 address at the edge switch specified by
   * column address.  Technically, an edge switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the edge switch desired
   *
   * \returns Ipv6Address of one of the interfaces of the edge switch
   *          column address
   */
  Ipv6Address GetEdgeSwitchIpv6Address (uint32_t col) const;

  /**
   * This returns an Ipv6 address at the aggregate switch specified by
   * column address.  Technically, an aggregate switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the aggregate switch desired
   *
   * \returns Ipv6Address of one of the interfaces of the aggregate switch
   *          column address
   */
  Ipv6Address GetAggregateSwitchIpv6Address (uint32_t col) const;

  /**
   * This returns an Ipv6 address at the core switch specified by
   * column address.  Technically, a core switch will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the lowest index server connected to this switch.
   *
   * \param col the column address of the core switch desired
   *
   * \returns Ipv6Address of one of the interfaces of the core switch
   *          column address
   */
  Ipv6Address GetCoreSwitchIpv6Address (uint32_t col) const;

  /**
   * This returns an Ipv4 address at the server specified by
   * the col address.  Technically, a server will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv4 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the leftmost edge switch  connected to this server.
   *
   * \param col the column address of the server desired
   *
   * \returns Ipv4Address of one of the interfaces of the server
   *          specified by the col address
   */
  Ipv4Address GetServerIpv4Address (uint32_t col) const;

  /**
   * This returns an Ipv6 address at the server specified by
   * the col address.  Technically, a server will have
   * multiple interfaces in the FatTree; therefore, it also has
   * multiple Ipv6 addresses.  This method only returns one of
   * the addresses. If you picture the FatTree, the address returned
   * is the leftmost edge switch connected to this server.
   *
   * \param col the column address of the server desired
   *
   * \returns Ipv6Address of one of the interfaces of the server
   *          specified by the col address
   */
  Ipv6Address GetServerIpv6Address (uint32_t col) const;

  /**
   * \param stack an InternetStackHelper which is used to install
   *              on every node in the FatTree
   */
  void InstallStack (InternetStackHelper stack);

  /**
   * Assigns Ipv4 addresses to all the interfaces of switches and hosts
   *
   * \param network an IPv4 address representing the network portion
   *                of the IPv4 Address
   *
   * \param mask the mask length
   */
  void AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask);

  /**
   * Assigns Ipv6 addresses to all the interfaces of the switches and hosts
   *
   * \param network an IPv6 address representing the network portion
   *                of the IPv6 Address
   *
   * \param prefix the prefix length
   */
  void AssignIpv6Addresses (Ipv6Address network, Ipv6Prefix prefix);

  /**
   * Sets up the node canvas locations for every node in the FatTree.
   * This is needed for use with the animation interface
   *
   * \param ulx upper left x value
   * \param uly upper left y value
   * \param lrx lower right x value
   * \param lry lower right y value
   */
  void BoundingBox (double ulx, double uly, double lrx, double lry);

private:
  uint32_t m_numPods;                                          //!< Number of pods
  std::vector<NetDeviceContainer> m_edgeSwitchDevices;         //!< Net Device container for edge switches and servers
  std::vector<NetDeviceContainer> m_aggregateSwitchDevices;    //!< Net Device container for aggregate switches and edge switches
  std::vector<NetDeviceContainer> m_coreSwitchDevices;         //!< Net Device container for core switches and aggregate switches
  Ipv4InterfaceContainer m_edgeSwitchInterfaces;               //!< IPv4 interfaces of edge switch
  Ipv4InterfaceContainer m_aggregateSwitchInterfaces;          //!< IPv4 interfaces of aggregate switch
  Ipv4InterfaceContainer m_coreSwitchInterfaces;               //!< IPv4 interfaces of core switch
  Ipv4InterfaceContainer m_serverInterfaces;                   //!< IPv4 interfaces of server
  Ipv6InterfaceContainer m_edgeSwitchInterfaces6;              //!< IPv6 interfaces of edge switch
  Ipv6InterfaceContainer m_aggregateSwitchInterfaces6;          //!< IPv6 interfaces of aggregate switch
  Ipv6InterfaceContainer m_coreSwitchInterfaces6;               //!< IPv6 interfaces of core switch
  Ipv6InterfaceContainer m_serverInterfaces6;                  //!< IPv6 interfaces of server
  NodeContainer m_coreSwitches;                                   //!< all the core switches in the fat-tree
  NodeContainer m_aggregateSwitches;                              //!< all the aggregate switches in the fat-tree
  NodeContainer m_edgeSwitches;                                   //!< all the edge switches in the fat-tree
  NodeContainer m_servers;                                        //!< all the servers in the fat-tree
};

} // namespace ns3

#endif /* POINT_TO_POINT_BCUBE_HELPER_H */
