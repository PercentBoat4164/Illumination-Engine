import setuptools
import sys
import Cython.Build


def cython_compile(file):
    sys.argv = [sys.argv[0], "build_ext", "--inplace"]
    print(Cython.Build.cythonize([file], language_level=3))

    setuptools.setup(ext_modules=Cython.Build.cythonize([file], language_level=3, quiet=True))
