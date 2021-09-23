# Rules for building with Meson

project(
  'lsinventory',
  'cpp',
  default_options: [
   'warning_level=3',
   'werror=true',
   'cpp_std=c++17',
  ],
  license: 'Apache-2.0',
)

version = vcs_tag(command: [ 'git', 'describe', '--always', '--dirty', '--long' ],
                  input: 'src/version.hpp.in',
                  output: 'version.hpp')

conf = configuration_data()
conf.set_quoted('MAPPER_SERVICE', get_option('mapper-service'))
conf.set_quoted('MAPPER_PATH', get_option('mapper-path'))
conf.set_quoted('MAPPER_IFACE', get_option('mapper-iface'))
conf.set_quoted('INVENTORY_PATH', get_option('inventory-path'))
conf.set_quoted('INVENTORY_IFACE', get_option('inventory-iface'))
conf.set('REMOTE_HOST_SUPPORT', get_option('remote-host-support').enabled())
configure_file(output: 'config.hpp', configuration: conf)

sdbusplus = dependency('sdbusplus', required: false)
if not sdbusplus.found()
    sdbusplus_proj = subproject('sdbusplus', required: true)
    sdbusplus = sdbusplus_proj.get_variable('sdbusplus_dep')
endif

if meson.get_compiler('cpp').has_header('nlohmann/json.hpp')
    nlohmann_json = declare_dependency()
else
    subproject('nlohmann', required: true)
    nlohmann_json = declare_dependency(
        include_directories: [
            'subproject/nlohmann/single_include',
            'subproject/nlohmann/single_include/nlohmann',
        ]
    )
endif

executable(
  'lsinventory',
  [
    version,
    'src/main.cpp',
    'src/inventory.cpp',
    'src/printer.cpp',
  ],
  dependencies: [
    sdbusplus,
    nlohmann_json,
  ],
  install: true
)

build_tests = get_option('tests')
if not build_tests.disabled()
    subdir('test')
endif
