#include "usdcard.h"
#include "usdcard_tests.h"

uint16 buffer[512];
uint16 abuffer[512];

uint8 usd_test_one_write_one_read_same_block()
{
	uint16 retv, i;

	for (i = 0; i < 512; i++)
	{
		buffer[i] = (uint16) 0x0041;
	}

	buffer[0] = 0x0042; //B
	buffer[1] = 0x0052; //R
	buffer[2] = 0x0041; //A
	buffer[3] = 0x0056; //V
	buffer[4] = 0x004F; //0

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }

	retv = usd_write_block(buffer, 0);

	if (retv) return 1;

	retv = usd_read_block(abuffer, 0);

	if (retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (buffer[i] != abuffer[i]) return 1;
	}

	return (retv) ? 1 : 0;
}

uint8 usd_test_two_read_same_block()
{
	uint16 retv, i;

	for (i = 0; i < 512; i++)
	{
		buffer[i] = (uint16) 0x0041;
	}

	buffer[0] = 0x0042; //B
	buffer[1] = 0x0052; //R
	buffer[2] = 0x0041; //A
	buffer[3] = 0x0056; //V
	buffer[4] = 0x004F; //0

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (buffer[i] != abuffer[i])
		{
			return 1;
		}
	}

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (buffer[i] != abuffer[i])
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_two_write_and_read_same_block()
{
	uint16 retv, i;

	for (i = 0; i < 512; i++){ buffer[i] = (uint16) 0x00FF; }
	retv = usd_write_block(buffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ buffer[i] = (uint16) 0x0042; }
	retv = usd_write_block(buffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0042)
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_two_write_and_read_different_blocks()
{
	uint16 retv, i;

	for (i = 0; i < 512; i++){ buffer[i] = (uint16) 0x0041; }
	retv = usd_write_block(buffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ buffer[i] = (uint16) 0x0042; }
	retv = usd_write_block(buffer, 1);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0041)
		{
			return 1;
		}
	}

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 1);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0042)
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_read_after_erase()
{
	uint16 retv, i;

	retv = usd_erase_blocks(0, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_write_erase_read()
{
	uint16 retv, i;

	for (i = 0; i < 512; i++){ buffer[i] = (uint16) 0x0041; }
	retv = usd_write_block(buffer, 0);

	if(retv) return 1;

	retv = usd_erase_blocks(0, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_erase_two_continuous_blocks()
{
	uint16 retv, i;

	retv = usd_erase_blocks(0, 1);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 0);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 1);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	return 0;
}

uint8 usd_test_erase_two_blocks()
{
	uint16 retv, i;

	retv = usd_erase_blocks(5, 5);

	if(retv) return 1;

	retv = usd_erase_blocks(6, 6);

	if(retv) return 1;

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 5);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	for (i = 0; i < 512; i++){ abuffer[i] = (uint16) 0xFFFF; }
	retv = usd_read_block(abuffer, 6);

	if(retv) return 1;

	for (i = 0; i < 512; i++)
	{
		if (abuffer[i] != 0x0000)
		{
			return 1;
		}
	}

	return 0;
}

int usd_unit_tests()
{
	uint8 failed = 0;

	failed += usd_test_one_write_one_read_same_block(); 	  //OK
	failed += usd_test_two_read_same_block(); 				  //OK
	failed += usd_test_two_write_and_read_same_block(); 	  //OK
	failed += usd_test_two_write_and_read_different_blocks(); //OK
	failed += usd_test_read_after_erase();					  //OK
	failed += usd_test_write_erase_read();					  //OK
	failed += usd_test_erase_two_continuous_blocks();		  //OK
	failed += usd_test_erase_two_blocks();					  //OK

	return (failed > 0) ? 1 : 0;
}
