# This file is a part of marzer/rt and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/rt/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

fs = import('fs')
exe_file_names = [
	'common',
	'soa',
	'main',
	'image',
	'back_buffer',
	'window',
	'camera',
	'scene',
	'colour',
	'renderer',
	'random'
]
exe_cpp_files = []
exe_extra_files = []
exe_extra_files += scene_files
exe_extra_files += global_extra_files
foreach name : exe_file_names
	foreach suffix : ['.hpp','.cpp','.toml','.natvis','.build']
		if fs.is_file(name + suffix)
			if suffix == '.cpp'
				exe_cpp_files += files(name + suffix)
			else
				exe_extra_files += files(name + suffix)
			endif
		endif
	endforeach
endforeach

subdir('renderers')

exe_dependencies = []
exe_dependencies += global_dependencies
exe_dependencies += subproject('muu', default_options: subproject_overrides).get_variable('muu_dep')
exe_dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2_dep')
exe_dependencies += subproject('sdl2', default_options: subproject_overrides).get_variable('sdl2main_dep')
exe_dependencies += subproject('xsimd', default_options: subproject_overrides).get_variable('xsimd_dep')
exe_dependencies += subproject('tomlplusplus', default_options: subproject_overrides).get_variable('tomlplusplus_dep')
exe_dependencies += subproject('argparse', default_options: subproject_overrides).get_variable('argparse_dep')
exe_dependencies += subproject('magic_enum', default_options: subproject_overrides).get_variable('magic_enum_dep')

exe_includes = []
exe_includes += vendor_includes

exe_args = []
exe_args += global_args

exe_link_args = []
exe_link_args += global_link_args

exe_overrides = []
exe_overrides += global_overrides
exe_overrides += [
	'warning_level=3',
	'werror=true'
]

exe = executable(
	meson.project_name(),
	exe_cpp_files,
	cpp_args: exe_args,
	link_args: exe_link_args,
	dependencies: exe_dependencies,
	override_options: exe_overrides,
	extra_files: exe_extra_files,
	include_directories: exe_includes
)
