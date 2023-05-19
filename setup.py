from setuptools import setup
from cmake_build_extension import BuildExtension, CMakeExtension

setup(
    name="pylibnfc",
    description="Python bindings for libnfc",
    url="https://github.com/pwuertz/pylibnfc",
    version="0.1",
    author="Peter Würtz",
    author_email="pwuertz@gmail.com",
    ext_modules=[CMakeExtension(
        name="pylibnfc",
        source_dir=".",
        install_prefix=".",
    )],
    cmdclass=dict(build_ext=BuildExtension),
)
