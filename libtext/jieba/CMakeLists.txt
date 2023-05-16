

turbo_cc_test(
        NAME
        keyword_extractor_test
        SRCS
        "keyword_extractor_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        pos_tagger_test
        SRCS
        "pos_tagger_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        pre_filter_test
        SRCS
        "pre_filter_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        segmentor_test
        SRCS
        "segmentor_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        segments_test
        SRCS
        "segments_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        trie_test
        SRCS
        "trie_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

turbo_cc_test(
        NAME
        textrank_test
        SRCS
        "textrank_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo
        GTest::gtest
        GTest::gtest_main
)

