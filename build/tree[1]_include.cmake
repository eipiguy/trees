if(EXISTS "C:/Users/ndnes/Dropbox/projects/code/branches/build/tree[1]_tests.cmake")
  include("C:/Users/ndnes/Dropbox/projects/code/branches/build/tree[1]_tests.cmake")
else()
  add_test(tree_NOT_BUILT tree_NOT_BUILT)
endif()