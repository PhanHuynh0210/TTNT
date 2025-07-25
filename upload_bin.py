import os
from SCons.Script import Import

Import("env")

def after_build(source, target, env):
    print("==> Running upload_bin.sh after build...")
    os.system("./upload_bin.sh")

env.AddPostAction("buildprog", after_build)
