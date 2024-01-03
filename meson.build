# This file is a part of marzer/rt and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/rt/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

project(
	'rt',
	'cpp',
	license : 'MIT',
	version : '0.1.0',
	meson_version : '>=0.63.0',
	default_options : [
		# https://mesonbuild.com/Builtin-options.html
		'buildtype=release',
		'default_library=shared',
		'b_lto=false',
		'b_ndebug=if-release',
		'warning_level=1',
		'werror=false',
		'cpp_std=c++20'
	]
)

#-----------------------------------------------------------------------------------------------------------------------
# global vars + imports
#-----------------------------------------------------------------------------------------------------------------------

is_debug = get_option('debug')
is_release = not is_debug
is_windows = host_machine.system() == 'windows'
is_x64 = host_machine.cpu_family() == 'x86_64'

cpp = meson.get_compiler('cpp')
is_gcc = cpp.get_id() == 'gcc'
is_clang = cpp.get_id() == 'clang'
is_msvc = cpp.get_id() == 'msvc'
is_icc_cl = cpp.get_id() == 'intel-cl'
is_icc = is_icc_cl or cpp.get_id() == 'intel'
is_lld = cpp.get_linker_id() == 'ld.lld'
has_exceptions = get_option('cpp_eh') != 'none'


#-----------------------------------------------------------------------------------------------------------------------
# global_args
#
# these are the arguments common to everything in the project
# *** they are not forwarded to dependents when using this as a submodule. ***
#-----------------------------------------------------------------------------------------------------------------------

global_args = []
# conformance and sane behaviour
global_args = cpp.get_supported_arguments(
	# clang/gcc
	'-ferror-limit=5',
	'-fmax-errors=5',
	'-Wno-unused-command-line-argument',
	'-Wno-reserved-macro-identifier',
	'-Wno-init-list-lifetime',
	'-fchar8_t',
	# msvc
	'/bigobj',
	'/Gy', # function-level linking
	'/GF', # string pooling
	'/openmp-',
	'/permissive-',
	'/utf-8',
	'/volatile:iso',
	'/Zc:__cplusplus',
	'/Zc:inline',
	'/Zc:externConstexpr',
	'/Zc:preprocessor',
	'/Zc:alignedNew',
	'/Zc:implicitNoexcept',
	'/Zc:noexceptTypes',
	'/Zc:nrvo',
	'/Zc:lambda',
	'/jmc-',
	'/Zi',
	'/ZH:SHA_256'
)
if is_msvc
	global_args += cpp.get_supported_arguments('/FS')
endif
if has_exceptions
	global_args += cpp.get_supported_arguments('/Zc:throwingNew', '-D_HAS_EXCEPTIONS=1')
else
	global_args += cpp.get_supported_arguments('-D_HAS_EXCEPTIONS=0')
endif
# warnings
global_args += cpp.get_supported_arguments(
	# clang
	'-Weverything',
	# gcc
	'-Wcast-align',
	'-Wcast-qual',
	'-Wctor-dtor-privacy',
	'-Wdisabled-optimization',
	'-Wfloat-equal',
	'-Wimport',
	'-Winit-self',
	'-Wlogical-op',
	'-Wmissing-declarations',
	'-Wmissing-field-initializers',
	'-Wmissing-format-attribute',
	'-Wmissing-include-dirs',
	'-Wmissing-noreturn',
	'-Wold-style-cast',
	'-Woverloaded-virtual',
	'-Wpacked',
	'-Wpointer-arith',
	'-Wredundant-decls',
	'-Wshadow',
	'-Wsign-conversion',
	'-Wsign-promo',
	'-Wstack-protector',
	'-Wstrict-null-sentinel',
	'-Wswitch-default',
	'-Wswitch-enum',
	'-Wundef',
	'-Wunreachable-code',
	'-Wunused',
	'-Wunused-parameter',
	'-Wuseless-cast',
	'-Wvariadic-macros',
	'-Wwrite-strings',
	'-Wmissing-noreturn',
	# msvc
	'/W4'
)
global_args += cpp.get_supported_arguments(
	'-Wno-c++98-compat',
	'-Wno-c++98-compat-pedantic',
	'-Wno-documentation',
	'-Wno-documentation-unknown-command',
	'-Wno-switch-enum',
	'-Wno-covered-switch-default',
	'-Wno-switch-default',
	'-Wno-padded',
	'-Wno-float-equal'
)
# optimization
global_args += cpp.first_supported_argument(
	'/arch:AVX2',
	'/arch:AVX',
	'/arch:SSE2'
)
global_args += cpp.first_supported_argument(
	'-mavx2',
	'-mavx',
	'-msse3',
	'-msse2',
)
global_args += cpp.get_supported_arguments(
	'/fp:fast',
	'/fp:except-',
	'-march=native',
	'-ffast-math',
	'-ffp-contract=fast',
	'-mfma'
)
if is_release
	global_args += cpp.get_supported_arguments(
		'/GL'
	)
endif

#-----------------------------------------------------------------------------------------------------------------------
# global_link_args
#
# these are the linker arguments common to everything in the projectwhen compiling shared libraries and executables.
# *** they are not forwarded to dependents when using this as a submodule. ***
#-----------------------------------------------------------------------------------------------------------------------

global_link_args = []
global_link_args += cpp.get_supported_link_arguments(
	# msvc
	'/DEBUG:FULL',
)
if is_release
	global_link_args += cpp.get_supported_link_arguments(
		# msvc
		'/OPT:REF,ICF=3',
		'/INCREMENTAL:NO',
	)
endif

#-----------------------------------------------------------------------------------------------------------------------
# global_dependencies
#
# these are the dependencies common to everything in the project
# *** they are not forwarded to dependents when using this as a submodule. ***
#-----------------------------------------------------------------------------------------------------------------------

global_dependencies = []

#-----------------------------------------------------------------------------------------------------------------------
# global_overrides
#
# these are the meson overrides common to everything in the project
# *** they are not forwarded to dependents when using this as a submodule. ***
#-----------------------------------------------------------------------------------------------------------------------

global_overrides = [ ]

subproject_overrides = [ 'werror=false', 'warning_level=0', 'default_library=static', 'buildtype=release' ]

#-----------------------------------------------------------------------------------------------------------------------
# subdirectories + files
#-----------------------------------------------------------------------------------------------------------------------

global_extra_files = files('README.md', 'LICENSE.txt', '.gitattributes', '.gitignore', '.clang-format', '.editorconfig')

subdir('vendor')
subdir('scenes')
subdir('src')

