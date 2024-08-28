load("@buildifier_prebuilt//:deps.bzl", "buildifier_prebuilt_deps")
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
load("@buildifier_prebuilt//:defs.bzl", "buildifier_prebuilt_register_toolchains")

def setup_buildifier_prebuilt():
    buildifier_prebuilt_deps()

    bazel_skylib_workspace()

    buildifier_prebuilt_register_toolchains()
