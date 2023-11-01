# This file, by virtue of the import statement below, merges
# the Tudat kernel module `tudatpy.kernel.astro.element_conversion` with
# its Python extensions defined in `tudatpy/kernel_hybrid/astro/element_conversion`.
# 
# This allows the import of all the C++ and Python submodules of the 
# `astro.element_conversion` kernel module directly from tudatpy:
# 
#     from tudatpy.astro.element_conversion import <any>
# 
# Without the statement below, importing the `astro.element_conversion` kernel module
# would only be possible as follows, and hybrid Python/C++ modules would not
# be posible in tudatpy.
# 
#     from tudatpy.kernel.astro.element_conversion import <any>
# 
# The reason why C++ kernel modules can only be imported as written above 
# is an issue with the `def_submodule` function of pybind11. The issue is discussed
# [here](https://github.com/pybind/pybind11/issues/2639). 
# 
# We circumvent the issue by automatically creating an *empty* Python module for each
# kernel module and submodule, and exposing the kernel module or submodule from the
# Python module by adding the import statement below to the Python module's `__init__.py` (this file). 
# This workaround was proposed in [this comment](https://github.com/pybind/pybind11/issues/2639#issuecomment-721238757).
# 
# An added benefit of this method is that it makes it possible to write Python extensions 
# and add them to the kernel modules simply by placing them inside this module!

from tudatpy.kernel.astro.element_conversion import *