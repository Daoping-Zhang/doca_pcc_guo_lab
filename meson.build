#
# Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
#
# This software product is a proprietary product of NVIDIA CORPORATION &
# AFFILIATES (the "Company") and all right, title, and interest in and to the
# software product, including all associated intellectual property rights, are
# and shall remain exclusively with the Company.
#
# This software product is governed by the End User License Agreement
# provided with the software product.
#

host_srcs = files([
	'host/' + APP_NAME + '.c',
	'host/' + APP_NAME + '_core.c',
])

device_code_build_script_name = 'build_device_code.sh'
device_code_build_script = meson.current_source_dir() / device_code_build_script_name

app_device_build_dir = meson.project_build_root() + '/' + APP_NAME + '/device/build_dpacc'

# Command to compile the application
app_builds = find_program(device_code_build_script)

doca_dep = dependency('doca-common')
doca_lib_dir = doca_dep.get_variable(pkgconfig : 'libdir')
pcc_app_dev_src_dir = meson.project_source_root() + '/' + APP_NAME + '/device/'

# check user option to enable TX bytes counter sampling
if get_option('enable_pcc_application_tx_counter_sampling')
	enable_tx_counter_sampling = 'true'
else
	enable_tx_counter_sampling = 'false'
endif

# check user option to enable NP RX rate update
if get_option('enable_pcc_application_np_rx_rate')
	enable_np_rx_rate = 'true'
else
	enable_np_rx_rate = 'false'
endif

# Compile DOCA PCC Reaction Point DPA program
pcc_rp_app_name = 'pcc_rp_app'
run_command(app_builds, pcc_app_dev_src_dir, app_device_build_dir, doca_lib_dir, pcc_rp_app_name,
	enable_tx_counter_sampling, enable_np_rx_rate, check: true)
pcc_rp_app = meson.get_compiler('c').find_library(pcc_rp_app_name, dirs : app_device_build_dir, static: true)

# Compile DOCA PCC Notification Point NIC Telemetry DPA program
pcc_np_nic_telemetry_app_name = 'pcc_np_nic_telemetry_app'
run_command(app_builds, pcc_app_dev_src_dir, app_device_build_dir, doca_lib_dir, pcc_np_nic_telemetry_app_name,
	enable_tx_counter_sampling, enable_np_rx_rate, check: true)
pcc_np_nic_telemetry_app = meson.get_compiler('c').find_library(pcc_np_nic_telemetry_app_name, dirs : app_device_build_dir, static: true)

# Build executable
doca_pcc_test = executable(DOCA_PREFIX + APP_NAME,
	host_srcs,
	c_args : base_c_args,
	include_directories: [app_inc_dirs, './host'],
	install_dir : app_install_dir,
	dependencies : [app_dependencies, pcc_rp_app, pcc_np_nic_telemetry_app],
	install: install_apps
)
