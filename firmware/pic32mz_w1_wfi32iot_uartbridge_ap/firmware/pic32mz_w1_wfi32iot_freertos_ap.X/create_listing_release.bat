SET REL_FILE=.\dist\pic32mz_w1_eth_wifi_freertos\production\pic32mz_w1_curiosity_freertos.X.production
SET TOOL="C:\Program Files\Microchip\xc32\v3.01\bin"

%TOOL%\xc32-readelf -s %REL_FILE%.elf > %REL_FILE%.sym
cmsort /S=7,100  %REL_FILE%.sym %REL_FILE%.symbols.txt
DEL %REL_FILE%.sym   
COPY %REL_FILE%.symbols.txt .
%TOOL%\xc32-objdump -S %REL_FILE%.elf > %REL_FILE%.disassembly.txt
COPY %REL_FILE%.disassembly.txt .
COPY %REL_FILE%.map .

break

SET DBG_FILE=.\dist\pic32mz_w1_eth_wifi_freertos\debug\pic32mz_w1_curiosity_freertos.X.debug
%TOOL%\xc32-readelf -s %DBG_FILE%.elf > %DBG_FILE%.sym
cmsort /S=7,100  %DBG_FILE%.sym %DBG_FILE%.symbols.txt
DEL %DBG_FILE%.sym   
COPY %DBG_FILE%.symbols.txt .
%TOOL%\xc32-objdump -S %DBG_FILE%.elf > %DBG_FILE%.disassembly.txt
COPY %DBG_FILE%.disassembly.txt .
COPY %DBG_FILE%.map .


