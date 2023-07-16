# This file is a part of marzer/rt and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/rt/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

cpp_files = files(
	'main.cpp',
	'window.cpp',
	'camera.cpp'
)

hpp_files = files(
	'common.hpp',
	'window.hpp',
	'camera.hpp',
	'soa.hpp'
)

misc_files = files(
	'soa.toml'
)

dependencies = []
dependencies += global_dependencies
dependencies += subproject('muu', default_options: subproject_overrides).get_variable('muu_dep')
dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2_dep')
dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2main_dep')

args = []
args += global_args

link_args = []
link_args += global_link_args

overrides = []
overrides += global_overrides
overrides += [
	'warning_level=3',
	'werror=true'
]

exe = executable(
	meson.project_name(),
	cpp_files + hpp_files,
	cpp_args: args,
	link_args: global_link_args,
	dependencies: dependencies,
	override_options: overrides,
	extra_files: misc_files
)
