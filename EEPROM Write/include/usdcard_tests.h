#ifndef INCLUDE_USDCARD_TESTS_H_
#define INCLUDE_USDCARD_TESTS_H_

uint8 usd_test_one_write_one_read_same_block();
uint8 usd_test_two_read_same_block();
uint8 usd_test_two_write_and_read_same_block();
uint8 usd_test_two_write_and_read_different_blocks();
uint8 usd_test_read_after_erase();
uint8 usd_test_write_erase_read();
uint8 usd_test_erase_two_continuous_blocks();
uint8 usd_test_erase_two_blocks();
int usd_unit_tests();

#endif /* INCLUDE_USDCARD_TESTS_H_ */
