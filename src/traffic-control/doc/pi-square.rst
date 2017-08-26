.. include:: replace.txt
.. highlight:: cpp

PI Square queue disc
--------------------

PI Square or PI2 [Schepper16]_ is a variant of PIE ([Pan13]_, [Pan16]_) whose behavior is intended
to be similar to that of PIE, but with reduced computation.

This chapter describes the PI Square queue disc implementation in |ns3|. 

Model Description
*****************

The source code for the PI2 model is located in the directory ``src/traffic-control/model``
and consists of 2 files `pi-square-queue-disc.h` and `pi-square-queue-disc.cc` defining a PiSquareQueueDisc class. The code was ported to |ns3| by Rohit P. Tahiliani based on the Linux
kernel code of PI2.  

* class :cpp:class:`PiSquareQueueDisc`: This class implements the main PI2 algorithm:

  * ``PiSquareQueueDisc::DoEnqueue ()``: This routine checks whether the queue is full, and if so, drops the packets and records the number of drops due to queue overflow. If queue is not full, this routine calls ``PiSquareQueueDisc::DropEarly()``, and depending on the value returned, the incoming packet is either enqueued or dropped.

  * ``PiSquareQueueDisc::DropEarly ()``: The decision to enqueue or drop the packet is taken by invoking this routine, which returns a boolean value; false indicates enqueue and true indicates drop.

  * ``PiSquareQueueDisc::CalculateP ()``: This routine is called at a regular interval of `m_tUpdate` and updates the drop probability, which is required by ``PiSquareQueueDisc::DropEarly()``

  * ``PiSquareQueueDisc::DoDequeue ()``: This routine calculates the average departure rate which is required for updating the drop probability in ``PiSquareQueueDisc::CalculateP ()``  

References
==========

.. [Schepper16] De Schepper, K., Bondarenko, O., Tsang, J., & Briscoe, B. (2016, November). PI2: A Linearized AQM for both Classic and Scalable TCP. In Proceedings of the 12th International on Conference on emerging Networking EXperiments and Technologies (pp. 105-119). ACM.`_.

.. [Pan13] Pan, R., Natarajan, P., Piglione, C., Prabhu, M. S., Subramanian, V., Baker, F., & VerSteeg, B. (2013, July). PIE: A lightweight control scheme to address the bufferbloat problem. In High Performance Switching and Routing (HPSR), 2013 IEEE 14th International Conference on (pp. 148-155). IEEE.  Available online at `<https://www.ietf.org/mail-archive/web/iccrg/current/pdfB57AZSheOH.pdf>`_.

.. [Pan16] R. Pan, P. Natarajan, F. Baker, G. White, B. VerSteeg, M.S. Prabhu, C. Piglione, V. Subramanian, Internet-Draft: PIE: A lightweight control scheme to address the bufferbloat problem, April 2016.  Available online at `<https://tools.ietf.org/html/draft-ietf-aqm-pie-07>`_.


Attributes
==========

The key attributes that the PiSquareQueueDisc class holds include the following: 

* ``Mode:`` PI2 operating mode (BYTES or PACKETS). The default mode is PACKETS. 
* ``QueueLimit:`` The maximum number of bytes or packets the queue can hold. The default value is 25 bytes / packets.
* ``MeanPktSize:`` Mean packet size in bytes. The default value is 1000 bytes.
* ``Tupdate:`` Time period to calculate drop probability. The default value is 30 ms. 
* ``Supdate:`` Start time of the update timer. The default value is 0 ms. 
* ``DequeueThreshold:`` Minimum queue size in bytes before dequeue rate is measured. The default value is 10000 bytes. 
* ``QueueDelayReference:`` Desired queue delay. The default value is 20 ms. 
* ``A:`` Value of alpha. The default value is 0.125.
* ``B:`` Value of beta. The default value is 1.25.

Examples
========

The example for PI2 is `pi-square-example.cc` which is located in
``src/traffic-control/examples``. To run the file (the first invocation
below shows the available command-line options):

:: 

   $ ./waf --run "pi-square-example --PrintHelp"
   $ ./waf --run pi-square-example
   $ ./waf --run "pi-square-example --useEcn=true"
   $ ./waf --run "pi-square-example --coupledAqm=true"

Validation
**********

The PI2 model is tested using :cpp:class:`PiSquareQueueDiscTestSuite`
class defined in `src/traffic-control/test/pi-square-queue-disc-test-suite.cc`.
The suite includes 4 test cases:

* Test 1: simple enqueue/dequeue with defaults, no drops
* Test 2: more data with defaults, unforced drops but no forced drops
* Test 3: same as test 2, but with higher QueueDelayReference
* Test 4: same as test 2, but with lesser dequeue rate
* Test 5: verify the working of Coupled AQM functionality

The test suite can be run using the following commands: 

::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf build
  $ ./test.py -s pi-square-queue-disc

or  

::

  $ NS_LOG="PiSquareQueueDisc" ./waf --run "test-runner --suite=pi-square-queue-disc"

