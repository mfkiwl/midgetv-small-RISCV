#-----------------------------------------------------------------------------
# Part of midgetv
# 2019. Copyright B. Nossum.
# For licence, see LICENCE
# -----------------------------------------------------------------------------

BIN = ../../../bin
OBJ = ../../../obj_dir

CFLAGS = -mabi=ilp32 -march=rv32i -nodefaultlibs -nostartfiles -Os \
	-Wno-unused-function -Wall \
	-DNOCSR=1 -DonlyEBR=1 -DSP_INITVAL=0x400 

all: $(BIN)/simeratosthenes \
	$(OBJ)/eratosthenes.lst \
	../../../hwtst/upduino2/ice40loaderprog.hv \
	../../../hwtst/max_upduino2/ice40loaderprog.hv \
	../../../hwtst/max_iceblink40-hx1k/ice40loaderprog.hv 


../../../hwtst/upduino2/ice40loaderprog.hv: $(OBJ)/eratosthenes.bin 
	../../../bin/midgetv_bin2ebr -i $< -o $@

../../../hwtst/max_upduino2/ice40loaderprog.hv: $(OBJ)/eratosthenes.bin 
	../../../bin/midgetv_bin2ebr -i $< -o $@

../../../hwtst/max_iceblink40-hx1k/ice40loaderprog.hv: $(OBJ)/eratosthenes.bin 
	../../../bin/midgetv_bin2ebr -i $< -o $@

$(BIN)/simeratosthenes:eratosthenes.c
	gcc -Wall -Wextra -Wno-unused-function -Dsim=1 -o $@ $< 

$(OBJ)/eratosthenes.o: | $(OBJ)/eratosthenes_crt0.o

.SECONDARY:

#$(OBJ)/%.S:%.c
#	riscv32-unknown-elf-gcc -S -fverbose-asm $(CFLAGS) -TEBRlink.ld  -I ./ -o $@ $<

$(OBJ)/%.o:%.c 
	riscv32-unknown-elf-gcc -c $(CFLAGS) -TEBRlink.ld  -I ./ -o $@ $<

$(OBJ)/%.o:%.S
	riscv32-unknown-elf-gcc -c $(CFLAGS) -TEBRlink.ld  -I ./ -I ../../inc/ -Wa,-a=$(OBJ)/$*.slst -o $@ $<

$(OBJ)/%.o:%.s
	riscv32-unknown-elf-gcc -c $(CFLAGS) -TEBRlink.ld  -I ./ -I ../../inc/ -Wa,-a=$(OBJ)/$*.slst -o $@ $<

%.out:%.o
	riscv32-unknown-elf-ld -TEBRlink.ld -nodefaultlibs -nostartfiles -o $@ $(OBJ)/eratosthenes_crt0.o $< -M > $(OBJ)/$(*F).map

%.bin:%.out
	riscv32-unknown-elf-objcopy -Obinary $< $@ 

%.lst:%.out
	riscv32-unknown-elf-objdump -M numeric,no-aliases -D $< > $@

clean:
	rm -f $(BIN)/simeratosthenes \
	$(OBJ)/eratosthenes_crt0.o \
	$(OBJ)/eratosthenes.bin \
	../../../hwtst/upduino2/ice40loaderprog.hv \
	../../../hwtst/max_upduino2/ice40loaderprog.hv \
	../../../hwtst/max_iceblink40-hx1k/ice40loaderprog.hv 

