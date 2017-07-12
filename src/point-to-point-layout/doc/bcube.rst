.. include:: replace.txt
.. highlight:: cpp

BCube topology
--------------

BCube is a server-centric network topology designed to meet the requirements of 
Modular Data Centres. It consists of servers with multiple network ports connected 
to multiple layers of COTS (commodity off-the-shelf) mini-switches. Servers act as 
not only end hosts, but also relay nodes for each other. BCube supports various 
bandwidth-intensive applications by speeding-up one-to-one, one-to-several, and 
one-to-all traffic patterns, and by providing high network capacity for all-to-all 
traffic. BCube exhibits graceful performance degradation as the server and/or switch 
failure rate increases. This property is of special importance for shipping-container 
data centers, since once the container is sealed and operational, it becomes very 
difficult to repair or replace its components.

Construction of BCube
---------------------

BCube is a recursively defined structure. There are two types of devices in BCube: 
Servers with multiple ports, and switches that connect a constant number of servers.  
A BCube0 is simply n servers connecting to an n-port switch. A BCube1 is constructed 
from n BCube0s and n n-port switches. More generically, a BCubek (k ≥ 1)) is constructed 
from n BCubek−1s and n^k n-port switches. Each server in a BCubek has k + 1 ports, which 
are numbered from level-0 to level-k. It is easy to see that a BCubek has N = n^(k+1) 
servers and k + 1 level of switches, with each level having n^k n-port switches.

The construction of a BCubek is as follows. We number the n BCubek−1s from 0 to n − 1 
and the servers in each BCubek−1 from 0 to n^k − 1. We then connect the level-k port 
of the i-th server (i ∈ [0, n^k − 1]) in the j-th BCubek−1 (j ∈ [0, n − 1]) to the j-th 
port of the i-th level-k switch. The links in BCube are bidirectional. In ns-3, all the
links  are created and configured using the associated ``PointToPointHelper`` object.

The BCube construction guarantees that switches only connect to servers and never 
directly  connect to other switches. As a direct consequence, we can treat the switches 
as dummy crossbars that connect several neighboring servers and let servers relay traffic 
for each other. 

Using the PointToPointBCube
----------------------------

The PointToPointBCubeHelper object can be instantiated by following statement.
  PointToPointBCubeHelper bcube (nLevels, nServers, pointToPointRouter);
  where,
  nLevels is number of levels (k)
  nServers is number of servers (n)
  pointToPointRouter is a ``PointToPointHelper`` object 

Examples
========
.
The BCube topology example is found at ``src/netanim/examples/bcube-animation.cc``.

References
**********

Link to the Paper: https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/comm136-guo.pdf
