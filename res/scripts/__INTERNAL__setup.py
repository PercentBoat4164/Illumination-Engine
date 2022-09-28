import setuptools
import sys
import Cython.Build


def compile(file):
    sys.argv = ["__INTERNAL__setup.py", "build_ext", "--inplace"]

    setuptools.setup(
        ext_modules=Cython.Build.cythonize([file])
    )


if __name__ == "__main__":
    compile(sys.argv[1])
