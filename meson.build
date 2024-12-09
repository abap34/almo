project('almo', 'cpp',
  version: 'v0.10.0-alpha',
  default_options: ['cpp_std=c++20'])

src = ['src/almo.cpp']
pybind_src = ['src/pyalmo.cpp']
test_src = ['tests/runtests.cpp']
bench_src = ['tests/runbench.cpp']


almo = executable('almo', src,
  include_directories: include_directories('src'),
  dependencies: []
)


gtest = dependency('gtest')
test_exe = executable('test_almo', test_src,
  include_directories: include_directories('src'),
  dependencies: [gtest]
)

test('test_almo', test_exe)


gbenchmark = dependency('benchmark')

benchmark_exe = executable('bench_almo', bench_src,
  include_directories: include_directories('src'),
  dependencies: [gbenchmark]
)

