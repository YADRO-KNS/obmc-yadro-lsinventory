# Rules for building with Meson

project('lsinventory', 'cpp',
        default_options: [
            'warning_level=3',
            'werror=true',
            'cpp_std=c++17',
        ],
        license: 'Apache-2.0',
        version: '2.0')

src = [ 'src/main.cpp',
        'src/inventory.cpp',
        'src/printer.cpp' ]

conf = configuration_data()
conf.set_quoted('MAPPER_SERVICE', get_option('mapper-service'))
conf.set_quoted('MAPPER_PATH', get_option('mapper-path'))
conf.set_quoted('MAPPER_IFACE', get_option('mapper-iface'))
conf.set_quoted('INVENTORY_SERVICE', get_option('inventory-service'))
conf.set_quoted('INVENTORY_PATH', get_option('inventory-path'))
conf.set_quoted('INVENTORY_IFACE', get_option('inventory-iface'))
configure_file(output: 'config.hpp', configuration: conf)

executable('lsinventory',
           src,
           dependencies: [
               dependency('sdbusplus'),
               dependency('json-c')
           ],
           install: true)
