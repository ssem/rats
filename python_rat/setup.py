#!/usr/bin/env python
from distutils.core import setup

setup(name="rat_server",
      version="alpha 1",
      description="rat server",
      author="ssem",
      author_email="ssem@tacnetsol.com",
      requires=[],
      scripts=["bin/client/connect",
               "bin/client/disconnect",
               "bin/client/upload",
               "bin/client/system",
               "bin/client/status",
               "bin/controller/controller"],
      packages=["rat_server"],)
