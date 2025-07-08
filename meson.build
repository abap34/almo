project('almo', 'cpp',
  version: 'v0.10.0-alpha',
  default_options: ['cpp_std=c++20'])

# Create version files as C++ string literals
git = find_program('git', required: false)
if git.found()
  version_txt = custom_target('version.txt',
    output: 'version.txt',
    command: [find_program('sh'), '-c', 'printf \'R"(%s)"\' "@0@" > @OUTPUT@'.format(meson.project_version())],
    build_by_default: true)
  
  commithash_txt = custom_target('commithash.txt',
    output: 'commithash.txt', 
    command: [find_program('sh'), '-c', 'printf \'R"(%s)"\' "$(@0@ show --format=%h --no-patch 2>/dev/null || echo development)" > @OUTPUT@'.format(git.full_path())],
    build_by_default: true)
else
  version_txt = custom_target('version.txt',
    output: 'version.txt',
    command: [find_program('sh'), '-c', 'printf \'R"(%s)"\' "@0@" > @OUTPUT@'.format(meson.project_version())],
    build_by_default: true)
    
  commithash_txt = custom_target('commithash.txt',
    output: 'commithash.txt',
    command: [find_program('sh'), '-c', 'printf \'R"(%s)"\' "development" > @OUTPUT@'],
    build_by_default: true)
endif

src = ['src/almo.cpp']
pybind_src = ['src/pyalmo.cpp']
test_src = ['tests/simple_test.cpp']
bench_src = ['tests/simple_bench.cpp']


almo = executable('almo', [src, version_txt, commithash_txt],
  include_directories: include_directories('src'),
  dependencies: []
)


gtest = dependency('gtest')
test_exe = executable('test_almo', [test_src, version_txt, commithash_txt],
  include_directories: include_directories('src'),
  dependencies: [gtest]
)

test('test_almo', test_exe)


gbenchmark = dependency('benchmark')

benchmark_exe = executable('bench_almo', [bench_src, version_txt, commithash_txt],
  include_directories: include_directories('src'),
  dependencies: [gbenchmark]
)

