
add_executable( bip_lock bip_lock.cpp )
target_link_libraries( bip_lock fc_pp )

add_executable( api api.cpp )
target_link_libraries( api fc_pp )

if( ECC_IMPL_PP STREQUAL pp_secp256k1 )
    add_executable( blind all_tests.cpp crypto/blind.cpp )
    target_link_libraries( blind fc_pp )
    target_include_directories(blind PUBLIC ${Boost_INCLUDE_DIR})
endif()

add_executable( task_cancel_test all_tests.cpp thread/task_cancel.cpp )
target_link_libraries( task_cancel_test fc_pp )


add_executable( bloom_test all_tests.cpp bloom_test.cpp )
target_link_libraries( bloom_test fc_pp )

add_executable( real128_test all_tests.cpp real128_test.cpp )
target_link_libraries( real128_test fc_pp )

add_executable( hmac_test hmac_test.cpp )
target_link_libraries( hmac_test fc_pp )

add_executable( blinding_test blinding_test.cpp )
target_link_libraries( blinding_test fc_pp )

add_executable( ecc_test crypto/ecc_test.cpp )
target_link_libraries( ecc_test fc_pp )

add_executable( log_test crypto/log_test.cpp )
target_link_libraries( log_test fc_pp )

#add_executable( test_aes aes_test.cpp )
#target_link_libraries( test_aes fc_pp ${rt_library} ${pthread_library} )
#add_executable( test_sleep sleep.cpp )
#target_link_libraries( test_sleep fc_pp )
#add_executable( test_rate_limiting rate_limiting.cpp )
#target_link_libraries( test_rate_limiting fc_pp )

add_executable( all_tests all_tests.cpp
                          compress/compress.cpp
                          crypto/aes_test.cpp
                          crypto/base_n_tests.cpp
                          crypto/bigint_test.cpp
                          crypto/blind.cpp
                          crypto/blowfish_test.cpp
                          crypto/dh_test.cpp
                          crypto/rand_test.cpp
                          crypto/sha_tests.cpp
                          network/http/websocket_test.cpp
                          thread/task_cancel.cpp
                          bloom_test.cpp
                          real128_test.cpp
                          utf8_test.cpp
                          )
target_link_libraries( all_tests fc_pp )

# Doesn't needed if we add hunter
target_include_directories(bip_lock PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(api PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(task_cancel_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(bloom_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(real128_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(hmac_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(blinding_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(ecc_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(log_test PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(all_tests PUBLIC ${Boost_INCLUDE_DIR})