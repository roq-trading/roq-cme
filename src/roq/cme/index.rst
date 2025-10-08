.. _roq-cme:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


roq-cme
=======

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-cme

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-cme


Description
-----------

:code:`roq-cme` is a gateway

* Using the MDP3 (SBE/multicast) interface for market data.

  * Supports MBOFD (Market by Order).
  * Using both A and B channels for fastest access and to lower the probability of packet loss.

* Using the iLink3 (SBE/TCP) interface for order management.


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        -
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        -
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |cross-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |negative-cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |negative-cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |cross-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.



Using
-----

.. code-block:: shell

   $ roq-cme [FLAGS]


.. _roq-cme-flags:

Flags
-----

.. code-block:: shell

   $ roq-cme --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: iLink

   .. include:: flags/ilink.rstinc

.. tab:: Multicast

   .. include:: flags/multicast.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc

.. tab:: Test

   .. include:: flags/test.rstinc


Environments
------------

.. code-block:: shell

  $ $CONDA_PREFIX/share/roq-cme/flags

There is no specific environment setup.

You need to download the MDP :code:`config.xml` and iLink :code:`MSGW_Config.xml` files.
These files are used to discover IP addresses for MDP multicast and iLink's MSGW (market segment gateways).

The gateway flags :code:`--multicast_channel_ids` and :code:`--ilink_market_segment_ids` are used to
join multicast streams and connect to specific MSGW end-points.

The multicast addresses must be made available from a local network interface.
The actual IP address of that interface must be provided with the :code:`--multicast_local_interface` flag.


Configuration
-------------

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-cme/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Order Management
----------------


Order Types
~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`MARKET`
    - Mapped to :code:`'MarketwithProtection'` (SBE)

  * - :cpp:class:`LIMIT`
    - Mapped to :code:`'Limit'` (SBE)


Time in Force
~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`GFD`
    - Mapped to :code:`'Day'` (SBE)

  * - :cpp:class:`GTC`
    - Mapped to :code:`'GoodTillCancel'` (SBE)

  * - :cpp:class:`IOC`
    - Mapped to :code:`'FillAndKill'` (SBE)

  * - :cpp:class:`FOK`
    - Mapped to :code:`'FillOrKill'` (SBE)


Comments
--------

* The OrderEntry (iLink) connections are established based on the provided list
  of market segments.
  This is a bit unusual: the list of accounts are normally 1:1 with the connections.
  It is only allowed to configure a single account until we know better how this
  all works with CME's party-definitions.

* The actual exchange (e.g. XCBT) is now used for order management.
  The reason is technical and due to most gateways only providing access to
  a single exchange.
  The exchange name used for creating, modify and canceling orders, must therefore
  match the name provided with the :code:`--exchange` flag.

* :code:`ExternalLatency` is currently only published when receiving a heartbeat from the exchange.

* :code:`MarketByOrder` is opt-in using the :code:`--enable_market_by_order` flag.

* :code:`ExecutionReportTradeOutright` reports :code:`LastPx` as the order's limit price.
  This should normally be expected to be the (average) price of the fill(s).


:code:`roq-cme-import`
======================

.. code-block:: shell

   $ roq-cme-import [FLAGS] PCAP_FILE


Description
-----------

:code:`roq-cme-import` is a tool to create a :code:`.roq` event-log by importing a :code:`.pcap` file.

* Requires the :code:`config.xml` file (from CME's FTP site) to map IP addresses to channels.
* Requires the :code:`secdef.dat` file (from CME's FTP site) if the :code:`.pcap` file does **not** contain the relevant instrument definitions.
* Requires the :code:`.pcap` file to contain all incremental messages from CME's start-up (Sunday) if the recovery channels are **not** available.
* Will not work if the :code:`.pcap` file has gaps and the recovery channels are **not** available.


Flags
-----

.. code-block:: shell

   $ roq-cme-import --help

.. tab:: Flags

   .. include:: import/flags/flags.rstinc

.. tab:: CME

   .. include:: import/flags/cme.rstinc

.. tab:: EventLog

   .. include:: import/flags/event_log.rstinc

.. tab:: Misc

   .. include:: import/flags/misc.rstinc

.. tab:: Test

   .. include:: import/flags/test.rstinc


Example
-------

.. code-block:: shell

   $ roq-cme-import \
       --name "cme" \
       --type "pcap" \
       --channel_ids 344 \
       --symbols "ZN[HMUZ][0-9]" \
       --cme_config_file "config.xml" \
       --cme_secdef_file "secdef.dat" \
       --event_log_output_file "test.roq" \
       all.pcap


:code:`roq-cme-filter`
======================

.. code-block:: shell

   $ roq-cme-filter [FLAGS]


Description
-----------

:code:`roq-cme-filter` is a tool to generate the PCAP filter required to capture specific channels.


Flags
-----

.. code-block:: shell

   $ roq-cme-filter --help

.. tab:: Flags

   .. include:: filter/flags/flags.rstinc

.. tab:: CME

   .. include:: filter/flags/cme.rstinc


Example
-------

.. code-block:: shell

   $ roq-cme-filter \
       --type "tcpdump" \
       --channel_ids 344 \
       --cme_config_file "config.xml"

   (host 224.0.31.110 and port 14344) or (host 224.0.32.110 and port 15344) or (host 224.0.31.68 and port 14344) or (host 224.0.32.68 and port 15344) or (host 224.0.31.89 and port 14344) or (host 224.0.32.89 and port 15344) or (host 233.72.75.33 and port 23344) or (host 233.72.75.96 and port 22344)(dev)


This will output a :code:`tcpdump` filter for :code:`channel_ids`.


References
----------


Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`

Exchange
~~~~~~~~

* `Website <https://www.cmegroup.com/>`__
* `FTP site <https://www.cmegroup.com/ftp/>`__
* `MDP3 Documentation <https://www.cmegroup.com/confluence/display/EPICSANDBOX/CME+MDP+3.0+Market+Data/>`__
* `iLink3 Documentation <https://www.cmegroup.com/confluence/display/EPICSANDBOX/iLink+3+Binary+Order+Entry/>`__
