# Rules for building tests

test(
  'inventory',
  executable(
    'lsinventory_test',
    [
      'inventory_test.cpp',
      '../src/inventory.cpp',
    ],
    dependencies: [
      dependency('gmock', disabler: true, required: build_tests),
      dependency('gtest', main: true, disabler: true, required: build_tests),
      dependency('sdbusplus'),
    ],
    include_directories: '../src',
  )
)

configure_file(output: 'config.hpp', configuration: conf)
