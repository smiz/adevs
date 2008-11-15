PREFIX=../..
include ../make.common

# Everything else should work fine
check: gcd_test gpt_test tokenring_test race_test fire_ca_test

gpt_test:
	cd gpt $(CMD_SEP) $(MAKE) check

gcd_test:
	cd gcd $(CMD_SEP) $(MAKE) check

tokenring_test:
	cd tokenring $(CMD_SEP) $(MAKE) check

race_test:
	cd race $(CMD_SEP) $(MAKE) check

fire_ca_test:
	cd fire_ca $(CMD_SEP) $(MAKE) check

clean_all:
	cd gcd $(CMD_SEP) $(MAKE) clean
	cd gpt $(CMD_SEP) $(MAKE) clean
	cd tokenring $(CMD_SEP) $(MAKE) clean
	cd race $(CMD_SEP) $(MAKE) clean
	cd fire_ca $(CMD_SEP) $(MAKE) clean_all
	$(MAKE) clean

