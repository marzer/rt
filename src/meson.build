# This file is a part of marzer/rt and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/rt/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

fs = import('fs')
file_names = [
	'common',
	'soa',
	'main',
	'window',
	'camera',
	'image_view',
	'scene',
]
cpp_files = []
extra_files = []
foreach name : file_names
	foreach suffix : ['.hpp','.cpp','.toml']
		if fs.is_file(name + suffix)
			if suffix == '.cpp'
				cpp_files += files(name + suffix)
			else
				extra_files += files(name + suffix)
			endif
		endif
	endforeach
endforeach


dependencies = []
dependencies += global_dependencies
dependencies += subproject('muu', default_options: subproject_overrides).get_variable('muu_dep')
dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2_dep')
dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2main_dep')
dependencies += subproject('xsimd', default_options: subproject_overrides).get_variable('xsimd_dep')
dependencies += subproject('tomlplusplus', default_options: subproject_overrides).get_variable('tomlplusplus_dep')

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
	cpp_files,
	cpp_args: args,
	link_args: global_link_args,
	dependencies: dependencies,
	override_options: overrides,
	extra_files: extra_files
)
