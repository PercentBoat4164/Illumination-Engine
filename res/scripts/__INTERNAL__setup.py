import setuptools
import sys
import Cython.Build


def cython_compile(file):
    sys.argv = ["__INTERNAL__setup.py", "build_ext", "--inplace", ]

    setuptools.setup(
        ext_modules=Cython.Build.cythonize([file], language_level=3)
    )
