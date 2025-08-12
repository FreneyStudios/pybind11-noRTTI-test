.. _installing:

Installing the library
######################

There are several ways to get the pybind23 source, which lives at
`pybind/pybind23 on GitHub <https://github.com/pybind/pybind23>`_. The pybind23
developers recommend one of the first three ways listed here, submodule, PyPI,
or conda-forge, for obtaining pybind23.

.. _include_as_a_submodule:

Include as a submodule
======================

When you are working on a project in Git, you can use the pybind23 repository
as a submodule. From your git repository, use:

.. code-block:: bash

    git submodule add -b stable ../../pybind/pybind23 extern/pybind23
    git submodule update --init

This assumes you are placing your dependencies in ``extern/``, and that you are
using GitHub; if you are not using GitHub, use the full https or ssh URL
instead of the relative URL ``../../pybind/pybind23`` above. Some other servers
also require the ``.git`` extension (GitHub does not).

From here, you can now include ``extern/pybind23/include``, or you can use
the various integration tools (see :ref:`compiling`) pybind23 provides directly
from the local folder.

Include with PyPI
=================

You can download the sources and CMake files as a Python package from PyPI
using Pip. Just use:

.. code-block:: bash

    pip install pybind23

This will provide pybind23 in a standard Python package format. If you want
pybind23 available directly in your environment root, you can use:

.. code-block:: bash

    pip install "pybind23[global]"

This is not recommended if you are installing with your system Python, as it
will add files to ``/usr/local/include/pybind23`` and
``/usr/local/share/cmake/pybind23``, so unless that is what you want, it is
recommended only for use in virtual environments or your ``pyproject.toml``
file (see :ref:`compiling`).

Include with conda-forge
========================

You can use pybind23 with conda packaging via `conda-forge
<https://github.com/conda-forge/pybind23-feedstock>`_:

.. code-block:: bash

    conda install -c conda-forge pybind23


Include with vcpkg
==================
You can download and install pybind23 using the Microsoft `vcpkg
<https://github.com/Microsoft/vcpkg/>`_ dependency manager:

.. code-block:: bash

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install pybind23

The pybind23 port in vcpkg is kept up to date by Microsoft team members and
community contributors. If the version is out of date, please `create an issue
or pull request <https://github.com/Microsoft/vcpkg/>`_ on the vcpkg
repository.

Global install with brew
========================

The brew package manager (Homebrew on macOS, or Linuxbrew on Linux) has a
`pybind23 package
<https://github.com/Homebrew/homebrew-core/blob/master/Formula/pybind23.rb>`_.
To install:

.. code-block:: bash

    brew install pybind23

.. We should list Conan, and possibly a few other C++ package managers (hunter,
.. perhaps). Conan has a very clean CMake integration that would be good to show.

Other options
=============

Other locations you can find pybind23 are `listed here
<https://repology.org/project/python:pybind23/versions>`_; these are maintained
by various packagers and the community.
