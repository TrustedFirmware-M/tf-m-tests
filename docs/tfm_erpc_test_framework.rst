############################
The TF-M eRPC Test Framework
############################

The TF-M eRPC Test Framework is a Remote Procedure Call (RPC) based test
framework written in order to have complex test cases running on a host system
which trigger the PSA client APIs on a connected target.
It is based on the `eRPC <https://github.com/EmbeddedRPC/erpc>`__
project. It is an additional framework to the existing one which runs NS test
suites entirely on the target. It enables you to run test codes on host
machines as if they were running on the device. It has the following advantages:

- Off-load test codes from device to host

  Arm Cortex-M based platform devices usually have limited flash storage which
  can only fit small test suites. With test codes running on hosts you can
  run far more tests than on the devices, within a richer test environment.

- Less frequent image downloading for test code development

  As the test codes run on the host, you don't need to update the image on
  device when you update test codes.

- Host can get test pass or failure directly from the return codes

  This would be helpful for test automation because the system which triggers
  the tests can read the results programmatically and not relying on parsing
  the UART logs like it happens for the current device-based tests.

****************
How Does It Work
****************
Usually, the NS tests are executed in the NSPE of the device. The NS image
on the device contains the test code, which calls into TF-M through the PSA
client APIs.

Using the eRPC test framework, the tests use a client-server architecture:
the NS test code is executed on the host machine as a client, while the NSPE on
the device implements the server side. The NSPE of device side does not run the
test code anymore. When the tests call the PSA client APIs, they call into the
eRPC framework. The eRPC framework handles the communication with the NSPE on
the device, which calls into TF-M through the PSA client APIs.

The prototypes of the PSA client APIs are the same while the implementations
are different. Refer to the following sections for more details.

The Structure
=============

The following diagram shows the software structure.

.. figure:: media/erpc_test_framework.svg

- eRPC Framework

  The eRPC (Embedded Remote Procedure Call) framework system.

- eRPC Client Shim

  The eRPC generated shim layer of remote APIs for clients. It serializes the
  identifier of the API and its parameters into a stream of bytes and
  transports to the server through a communication channel such as
  UART and TCP/IP. The codes are generated by the `erpcgen tool <https://github.com/EmbeddedRPC/erpc/wiki/erpcgen>`_.
  To re-generate the shim code for server and client side:

  .. code-block:: bash

    cd <tf-m-tests/erpc>
    erpcgen -o generated_files/ -v tfm.erpc

- eRPC Server Shim

  The generated shim layer of the server. It registers a callback function to
  the eRPC framework. When the framework receives any requests from the client,
  it calls the callback function. The callback deserializes the byte stream
  to determine what API to call and then invokes it with the corresponding
  parameters from the byte stream. And then it returns the results to the
  client in the reverse routine.

- API Wrapper

  Part of the parameters of ``psa_call`` API is not supported by eRPC directly,
  thus an API wrapper is required to transform the ``in_vec/out_vec``
  structures to eRPC supported data types. The wrapper API is named as
  ``erpc_psa_call``. On the client side, the wrapper implements the
  ``psa_call`` which calls the ``erpc_psa_call`` in the client shim layer.
  On the server side, the wrapper implements the ``erpc_psa_call`` which is
  called by the shim layer. The ``erpc_psa_call`` then calls the ``psa_call``.

- Test Suites

  It can be the existing TF-M regression tests or any other tests that interact
  with TF-M using the PSA Client APIs.

- Host App

  Initializes the eRPC client and starts the test suites.

- Target App

  Initializes the eRPC server and listens for requests from the eRPC client.

Supported APIs
==============

The APIs supported for doing RPC are the PSA Client APIs because they are the
lowest level APIs that interact with TF-M. You can build lots of test suites
upon them. You can also add your own APIs in the ``tfm.erpc`` file.
Please refer to `IDL Reference <https://github.com/EmbeddedRPC/erpc/wiki/IDL-Reference>`_
for the syntax of the file.

API Grouping
************

PSA Client APIs are categorized into common APIs and connection-based service
APIs. Connection-based APIs can only be used if there are connection-based
services enabled in TF-M, otherwise they return with an error code.

Common APIs:
  - psa_framework_version()
  - psa_version()
  - psa_call()

Connection-based specific APIs:
  - psa_connect()
  - psa_close()

Transportation layer
====================

On the device side, only UART transportation is supported in NSPE being the
most common peripheral that is generally available on all supported platforms.
For the host side, both UART and TCP are available. The main use case of the
TCP transport from the host side is to interface with Fast Models (FVPs) where
the UART data is getting transferred between a TCP/IP socket of the host and a
serial port of the target. This happens with the help of a virtual component
of the model called the TelnetTerminal which acts as a gateway between the host
and the target. See the `Fast Models reference guide
<https://developer.arm.com/documentation/100966/1116/Getting-Started-with-Fixed-Virtual-Platforms/Using-a-terminal-with-a-system-model>`_
for more details.

********************
Platform Integration
********************

As a basic requirement the platform must provide a UART driver that is
compliant with the `CMSIS-Driver specification
<https://www.keil.com/pack/doc/CMSIS_Dev/Driver/html/index.html>`_ and support
the enablement of the receiver and transmitter side of the peripheral via the
``ARM_USART_Control()`` CMSIS API. These requirements are coming from the fact
that the UART transport implementation `in eRPC
<https://github.com/EmbeddedRPC/erpc/blob/1.9.1/erpc_c/transports/erpc_uart_cmsis_transport.h>`_
is based on CMSIS-Driver APIs.

Secondly, platforms need to create their folders under the ``erpc/platform``
and then create the ``config_erpc_target.h`` to specify the UART port to be
used for eRPC transportation.

.. code-block::

  #define ERPC_UART           Driver_USART0

.. note::

  The folder structure in ``erpc/platform`` must be the same as the
  ``platform/ext/target`` of the TF-M repository.

It is recommended to use a different UART than the one used for stdio. If the
same UART device is used for both, then the TF-M logs (both SPM and
Secure Partitions) and any log messages from the NSPE must be disabled.
Otherwise, the eRPC transportation might fail. This happens when the eRPC
messages are intermixed with other data (e.g. log messages) on the same UART
device ultimately corrupting eRPC communication.

***********************
Application Integration
***********************

The TF-M eRPC test framework provides two CMake libraries for integration.
One is the ``erpc_client``, the other is the ``erpc_server``. Both include the
eRPC framework, the shim layers, API wrappers and expose an initialization API
for the client and server respectively.

The initialization does not include the initialization of the transportation
layer as it is use case specific which kind of transportation is used. It is
the responsibility of the client and server applications to initialize the
transportation layers and pass them to the ``erpc_client`` and ``erpc_server``
components.

TF-M provides the ``app/erpc_app.c`` as the default server application which
first initializes a CMSIS UART transport and then starts the eRPC server.

The ``CONFIG_TFM_ERPC_TEST_FRAMEWORK`` configuration option is provided to
enable the eRPC framework on the device (server) side. The default server
application will be automatically built and developers will only need to focus
on the client application development.

In summary, on the server side the only requirement is to enable
``CONFIG_TFM_ERPC_TEST_FRAMEWORK`` option at build time. On the client side,
one must:

- Initialize the transportation layer using eRPC provided APIs,
- Call the eRPC client initialization function provided by TF-M's eRPC test
  framework with the transportation instance initialized above
  as its parameter,
- Develop the application code,
- Building with CMake:

  - ``add_subdirectory`` with the ``erpc/client``,
  - link the ``erpc_client`` library to the application.

For more information on how to build and run the TF-M regression test suites
on various platforms please refer to :doc:`this document <tfm_erpc_test_build_and_run>`.
There is also an example of a simple host application at
``erpc/host_example`` for reference.

.. toctree::
  :caption: Examples
  :titlesonly:
  :hidden:

  eRPC Test Build and Run <tfm_erpc_test_build_and_run>

--------------

*Copyright (c) 2023-2024, Arm Limited. All rights reserved.*
