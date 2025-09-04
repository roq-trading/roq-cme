.. _roq-cme:

.. |checkmark| unicode:: U+2713

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


:code:`roq-cme`
---------------

.. code-block:: shell

   $ roq-cme [FLAGS]


Description
~~~~~~~~~~~

:code:`roq-cme` is a gateway

* Using the MDP3 (SBE/multicast) interface for market data.

  * Supports MBOFD (Market by Order).
  * Using both A and B channels for fastest access and to lower the probability of packet loss.

* Using the iLink3 (SBE/TCP) interface for order management.


Supports
~~~~~~~~

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        -
      * - Futures
        - |checkmark|
      * - Options
        -
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        - |checkmark|
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|
      * - Time Series
        -

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        - |checkmark|
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        -

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        -
      * - Funds
        -


.. _roq-cme-flags:

Flags
~~~~~

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
~~~~~~~~~~~~

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
~~~~~~~~~~~~~

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-cme/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
~~~~~~~~~~~

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Incremental
      - MDInstrumentDefinitionFuture54, MDInstrumentDefinitionOption55, MDInstrumentDefinitionSpread56,
        MDInstrumentDefinitionFixedIncome57, MDInstrumentDefinitionRepo58, MDInstrumentDefinitionFX63
      -

    * - :cpp:class:`roq::MarketStatus`
      - Incremental
      - SecurityStatus30,
        MDInstrumentDefinitionFuture54, MDInstrumentDefinitionOption55, MDInstrumentDefinitionSpread56,
        MDInstrumentDefinitionFixedIncome57, MDInstrumentDefinitionRepo58, MDInstrumentDefinitionFX63
      -

    * - :cpp:class:`roq::TopOfBook`
      - Incremental
      - MDIncrementalRefreshBook46
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - Incremental
      - MDIncrementalRefreshBook46
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      - Incremental
      - MDIncrementalRefreshBook46, MDIncrementalRefreshOrderBook47
      -

    * - :cpp:class:`roq::TradeSummary`
      - Incremental
      - MDIncrementalRefreshTradeSummary48,
        MDIncrementalRefreshTradeSummaryLongQty65
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - Incremental
      - MDIncrementalRefreshDailyStatistics49, MDIncrementalRefreshSessionStatistics51,
        MDIncrementalRefreshSessionStatisticsLongQty67, MDIncrementalRefreshVolume37
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Instrument Definition
      - MDInstrumentDefinitionFuture54, MDInstrumentDefinitionOption55, MDInstrumentDefinitionSpread56,
        MDInstrumentDefinitionFixedIncome57, MDInstrumentDefinitionRepo58, MDInstrumentDefinitionFX63
      -

    * - :cpp:class:`roq::MarketStatus`
      - Instrument Definition
      - MDInstrumentDefinitionFuture54, MDInstrumentDefinitionOption55
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MBP Market Recovery
      - SnapshotFullRefresh52, SnapshotFullRefreshLongQty69
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      - MBO Market Recovery
      - SnapshotFullRefreshOrderBook53
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -


Statistics
^^^^^^^^^^

TBD


Order Management
~~~~~~~~~~~~~~~~

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - OrderEntry
      - ExecutionReportTradeOutright525, ExecutionReportTradeSpread526, ExecutionReportTradeSpreadLeg527
      -

    * - :cpp:class:`roq::TradeUpdate`
      - OrderEntry
      - ExecutionReportTradeOutright525, ExecutionReportTradeSpread526, ExecutionReportTradeSpreadLeg527
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - OrderEntry
      - ExecutionReportStatus532
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      - Unavailable

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      - OrderEntry
      - NewOrderSingle514
      -

    * - :cpp:class:`roq::ModifyOrder`
      - OrderEntry
      - OrderCancelReplaceRequest515
      -

    * - :cpp:class:`roq::CancelOrder`
      - OrderEntry
      - OrderCancelRequest516
      -

    * - :cpp:class:`roq::CancelAllOrders`
      - OrderEntry
      - OrderMassActionRequest529
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      - OrderUpdate
      - ExecutionReportNew522,  ExecutionReportReject523, ExecutionReportModify531, ExecutionReportCancel534, BusinessReject521
      -


Order Types
^^^^^^^^^^^

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
^^^^^^^^^^^^^

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


Position Effect
^^^^^^^^^^^^^^^

.. note::

  Not supported


Execution Instructions
^^^^^^^^^^^^^^^^^^^^^^

.. note::

  Not supported


Templates
^^^^^^^^^

.. note::

  Not supported


Account Management
~~~~~~~~~~~~~~~~~~

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      - Unavailable

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      - Unavailable


Streams
~~~~~~~

.. tab:: Instrument Definition

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - UDP
      - Primary purpose

        * Reference data

.. tab:: MBP Market Recovery

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - UDP
      - Primary purpose

        * MbP snapshot

.. tab:: MBO Market Recovery

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - UDP
      - Primary purpose

        * MbO snapshot

.. tab:: Incremental

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - UDP
      - Primary purpose

        * MbP incremental

.. tab:: OrderEntry (iLink)

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - TCP
      - Primary purpose

        * iLink


Constraints
~~~~~~~~~~~

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


Comments
~~~~~~~~

* :code:`ExternalLatency` is currently only published when receiving a heartbeat from the exchange.

* :code:`MarketByOrder` is opt-in using the :code:`--enable_market_by_order` flag.

* :code:`ExecutionReportTradeOutright` reports :code:`LastPx` as the order's limit price.
  This should normally be expected to be the (average) price of the fill(s).


:code:`roq-cme-import`
----------------------

.. code-block:: shell

   $ roq-cme-import [FLAGS] PCAP_FILE


Description
~~~~~~~~~~~

:code:`roq-cme-import` is a tool to create a :code:`.roq` event-log by importing a :code:`.pcap` file.

* Requires the :code:`config.xml` file (from CME's FTP site) to map IP addresses to channels.
* Requires the :code:`secdef.dat` file (from CME's FTP site) if the :code:`.pcap` file does **not** contain the relevant instrument definitions.
* Requires the :code:`.pcap` file to contain all incremental messages from CME's start-up (Sunday) if the recovery channels are **not** available.
* Will not work if the :code:`.pcap` file has gaps and the recovery channels are **not** available.


Flags
~~~~~

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
~~~~~~~

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
----------------------

.. code-block:: shell

   $ roq-cme-filter [FLAGS]


Description
~~~~~~~~~~~

:code:`roq-cme-filter` is a tool to generate the PCAP filter required to capture specific channels.


Flags
~~~~~

.. code-block:: shell

   $ roq-cme-filter --help

.. tab:: Flags

   .. include:: filter/flags/flags.rstinc

.. tab:: CME

   .. include:: filter/flags/cme.rstinc


Example
~~~~~~~

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

CME
~~~

* `Website <https://www.cmegroup.com/>`__
* `FTP site <https://www.cmegroup.com/ftp/>`__
* `MDP3 Documentation <https://www.cmegroup.com/confluence/display/EPICSANDBOX/CME+MDP+3.0+Market+Data/>`__
* `iLink3 Documentation <https://www.cmegroup.com/confluence/display/EPICSANDBOX/iLink+3+Binary+Order+Entry/>`__
