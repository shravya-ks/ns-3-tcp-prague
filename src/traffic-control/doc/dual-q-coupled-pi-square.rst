.. include:: replace.txt
.. highlight:: cpp

Dual Queue PI Square queue disc
-------------------------------

DualQ Coupled PI Square [Schepper17]_ is a DualQ Coupled AQM algorithm
based on the PI2 algorithm [Schepper16]_ for the Classic AQM. PI2 is an
improved variant of the PIE AQM [Pan16]_.

This chapter describes the DualQ Coupled PI Square queue disc implementation
in |ns3|. 

Model Description
*****************

The source code is located in ``src/traffic-control/model`` and consists of
2 files: `dual-q-coupled-pi-square-queue-disc.h` and `dual-q-coupled-pi-square-queue-disc.cc`
defining a DualQCoupledPiSquareQueueDisc class. The code was ported to |ns3|
by Shravya K.S. based on the IETF draft [Schepper17]_.  

* class :cpp:class:`DualQCoupledPiSquareQueueDisc`: This class implements the main
algorithm:

  * ``DualQCoupledPiSquareQueueDisc::DoEnqueue ()``: This routine checks whether
  the queue is full, and if so, drops the packets and records the number of
  drops due to queue overflow. If queue is not full, this routine calls the
  ``QueueDisc::IsL4S()'' method to check if a packet is of type Classic or L4S
  and enqueues it in appropriate queue.

  * ``DualQCoupledPiSquareQueueDisc::CalculateP ()``: This routine is called at
  a regular interval of `m_tUpdate` and updates the classic drop probability
  and L4S drop probability, which is required by
  ``DualQCoupledPiSquareQueueDisc::DoDequeue()``

  * ``DualQCoupledPiSquareQueueDisc::DoDequeue ()``: This routine schedules one packet
  for dequeuing (or zero if the queue is empty).  It also makes all the AQM
  decisions on dropping and marking. It is contained within a large while loop
  so that if it decides to drop a packet, it will continue until it selects a
  packet to schedule. It implements time-shifted FIFO scheduling. It takes the
  packet that waited the longest, biased against the Classic traffic by a time-
  shift of tshift. If an L4S packet is scheduled, the packet is marked if either
  the L4S threshold is exceeded, or if a random marking decision is drawn
  according to k times the probability p. If a Classic packet is scheduled,
  the packet is dropped or marked(if ECN capable) based on the squared
  probability p^2.

References
==========

.. [Schepper17] De Schepper, K., Briscoe, B. Bondarenko, O., & Tsang, I,Internet-Draft: DualQ Coupled AQM for Low Latency, Low Loss and Scalable Throughput, July 2017. Available online at `<https://tools.ietf.org/html/draft-ietf-tsvwg-aqm-dualq-coupled-01>`_.

.. [Schepper16] De Schepper, K., Bondarenko, O., Tsang, I., & Briscoe, B. (2016, November). PI2: A Linearized AQM for both Classic and Scalable TCP. In Proceedings of the 12th International on Conference on emerging Networking Experiments and Technologies (pp. 105-119). ACM.`_.

.. [Pan16] R. Pan, P. Natarajan, F. Baker, G. White, B. VerSteeg, M.S. Prabhu, C. Piglione, V. Subramanian, Internet-Draft: PIE: A lightweight control scheme to address the bufferbloat problem, April 2016.  Available online at `<https://tools.ietf.org/html/draft-ietf-aqm-pie-07>`_.


Attributes
==========

The key attributes that the DualQCoupledPiSquareQueueDisc class holds include the following: 

* ``Mode:`` DualPI2 operating mode (BYTES or PACKETS). The default mode is PACKETS. 
* ``QueueLimit:`` The maximum number of bytes or packets the queue can hold.
The default value is 25 bytes / packets.
* ``MeanPktSize:`` Mean packet size in bytes. The default value is 1000 bytes.
* ``Tupdate:`` Time period to calculate drop probability. The default value is 16 ms. 
* ``Supdate:`` Start time of the update timer. The default value is 0 ms. 
* ``A:`` Value of alpha. The default value is 10.
* ``B:`` Value of beta. The default value is 100.
* ``ClassicQueueDelayReference:`` Desired queue delay of Classic traffic.
The default value is 15 ms.
* ``L4SMarkThresold:`` L4S marking threshold in Time. The default value is 1ms.
* ``K:`` Coupling Factor. The default value is 2.

Examples
========

The example for DualPI2 is `dual-q-coupled-pi-square-example.cc` located in
``src/traffic-control/examples``. To run the file (the first invocation below
shows the available command-line options):

:: 

   $ ./waf --run "dual-q-coupled-pi-square-example --PrintHelp"
   $ ./waf --run "dual-q-coupled-pi-square-example"

Validation
**********

The DualPI2 model is tested using :cpp:class:`DualQCoupledPiSquareQueueDiscTestSuite`
class defined in `src/traffic-control/test/dual-q-coupled-pi-square-queue-disc-test-suite.cc`. The suite includes 4 test cases:

* Test 1: simple enqueue/dequeue with defaults, no drops
* Test 2: more packets of both L4S and Classic with L4S having higher marks than Classic
* Test 3: Pump only Classic packets
* Test 4: Pump only L4S packets

The test suite can be run using the following commands: 

::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf build
  $ ./test.py -s dual-q-coupled-pi-square-queue-disc

or  

::

  $ NS_LOG="DualQCoupledPiSquareQueueDisc" ./waf --run "test-runner --suite=dual-q-coupled-pi-square-queue-disc"
