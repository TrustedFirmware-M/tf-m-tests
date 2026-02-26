########################
Trusted Firmware-M Tests
########################
The Trusted Firmware-M(TF-M) Tests repo is meant to hold various tests for the
`Trusted Firmware-M`_.
The TF-M tests mainly focus on functionalities of various TF-M components such
as the TF-M SPM and various Secure Partitions.

There is also the `psa-arch-tests`_ suite which mainly focuses on the
implementation compliance of the Platform Security Architecture (PSA).

****************
Folder Structure
****************
- app_broker - Common application code that executes the testing, included by the TF-M tests and
  the `psa-arch-tests`_.
- cmake - Common CMake utility scripts.
- docs - The documents about test developments.
- erpc - eRPC test framework. Can be used to trigger tests at PSA Developer API
  level with test code running on a remote host system.
- lib - TF-M libraries and 3rd-party libraries. May contain either imported source codes or CMake
  files to fetch the projects.
- tests_psa_arch - PSA Arch tests integration with TF-M.
- tests_reg - TF-M regression test codes including test framework, test suites and test services.

#######
License
#######
This software is provided under the `BSD-3-Clause license <license.rst>`_.
Contributions to this project are accepted under the same license,
with developer sign-off as described in the `Contribution guidelines`_.

The ``lib/ext`` folder contains 3rd party projects and files with other
licenses as listed below. The original license texts are included in those folders.
Below is the list that are different from the BSD-3-Clause and may be a part of the
runtime image. The source code for these projects except ``CMSIS`` is fetched
from upstream at build time only.

``/lib/ext/CMSIS``
  `Apache License v2.0
  <https://github.com/ARM-software/CMSIS_5/blob/develop/LICENSE.txt>`_.
``/lib/ext/qcbor``
  `Modified BSD-3-Clause license
  <https://github.com/laurencelundblade/QCBOR/blob/master/LICENSE>`_


####################
Feedback and support
####################
Feedback can be submitted via email to
`TF-M mailing list <tf-m@lists.trustedfirmware.org>`__.

.. _Trusted Firmware-M: https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git/
.. _psa-arch-tests: https://github.com/ARM-software/psa-arch-tests
.. _Contribution guidelines: https://trustedfirmware-m.readthedocs.io/en/latest/contributing/contributing_process.html

--------------

*Copyright (c) 2020-2026, Arm Limited. All rights reserved.*
