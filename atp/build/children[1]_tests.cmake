add_test( ChildTest.ChildIsCreatedWithDefaultValues C:/Users/ndnes/Dropbox/projects/code/branches/atp/build/Debug/children.exe [==[--gtest_filter=ChildTest.ChildIsCreatedWithDefaultValues]==] --gtest_also_run_disabled_tests)
set_tests_properties( ChildTest.ChildIsCreatedWithDefaultValues PROPERTIES WORKING_DIRECTORY C:/Users/ndnes/Dropbox/projects/code/branches/atp/build SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
add_test( ChildTest.GetIndexAboveRangeGivesNull C:/Users/ndnes/Dropbox/projects/code/branches/atp/build/Debug/children.exe [==[--gtest_filter=ChildTest.GetIndexAboveRangeGivesNull]==] --gtest_also_run_disabled_tests)
set_tests_properties( ChildTest.GetIndexAboveRangeGivesNull PROPERTIES WORKING_DIRECTORY C:/Users/ndnes/Dropbox/projects/code/branches/atp/build SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
add_test( ChildTest.GetNegativeIndexGivesNull C:/Users/ndnes/Dropbox/projects/code/branches/atp/build/Debug/children.exe [==[--gtest_filter=ChildTest.GetNegativeIndexGivesNull]==] --gtest_also_run_disabled_tests)
set_tests_properties( ChildTest.GetNegativeIndexGivesNull PROPERTIES WORKING_DIRECTORY C:/Users/ndnes/Dropbox/projects/code/branches/atp/build SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set( children_TESTS ChildTest.ChildIsCreatedWithDefaultValues ChildTest.GetIndexAboveRangeGivesNull ChildTest.GetNegativeIndexGivesNull)