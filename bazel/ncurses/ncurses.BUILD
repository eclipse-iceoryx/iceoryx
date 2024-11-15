load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "ncurses",
    args = ["-j"],
    configure_options = [
        "--without-debug",
        "--without-ada",
        "--without-tests",
        "--enable-overwrite",
    ],
    lib_source = ":all_srcs",
    out_static_libs = [
        "libncurses.a",
        "libcurses.a",
    ],
    visibility = ["//visibility:public"],
)
