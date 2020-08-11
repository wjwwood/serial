load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
    name = "serial",
    deps = [
        ":serial_facade",
        ":common_impl",
    ] + select({
        ":windows": [":windows_impl"],
        ":linux": [":linux_impl"],
        ":macos": [":unix_impl"],
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "serial_facade",
    hdrs = glob(["**/*.h"]),
    includes = ["include"],
)

cc_library(
    name = "windows_impl",
    srcs = [
        "src/impl/list_ports/list_ports_win.cc",
        "src/impl/win.cc",
    ],
    deps = [":serial_facade"],
)

cc_library(
    name = "linux_impl",
    srcs = [
        "src/impl/list_ports/list_ports_linux.cc",
        "src/impl/unix.cc",
    ],
    deps = [":serial_facade"],
)

cc_library(
    name = "common_impl",
    srcs = ["src/serial.cc"],
    deps = [":serial_facade"],
)

cc_binary(
    name = "example",
    srcs = ["examples/serial_example.cc"],
    deps = [":serial"],
)

config_setting(
    name = "windows",
    constraint_values = ["@platforms//os:windows"],
)

config_setting(
    name = "linux",
    constraint_values = ["@platforms//os:linux"],
)

config_setting(
    name = "macos",
    constraint_values = ["@platforms//os:macos"],
)
