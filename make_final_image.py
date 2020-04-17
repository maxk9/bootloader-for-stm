#!/usr/bin/python
import sys
import glob
import binascii
import inspect
import os
from intelhex import bin2hex

# prepare images from projects
btl_const = 'Bootloader\\Debug\\Exe\\PC_Bootloader.bin'
appl = 'Appl\\Debug\\Exe\\FPC_mcu.bin'

#name of finally image
filename_bin = 'pulse_counter_full.bin'
filename_hex = 'pulse_counter_full.hex'

def write_bin(filename,data,offset):
    try:
        f = open(filename,'r+b')
    except IOError:
        f = open(filename,'wb')
    f.seek(offset)
    f.write(data)
    f.close()

#===========================================================================
if __name__ == '__main__':
    print("************************build image****************************")

try:
    #section of const bootloader
    with open(btl_const, mode='rb') as file: # b is important -> binary
        fileContent = file.read()
        write_bin(filename_bin, fileContent, 0)

    
    #section of application
    with open(appl, mode='rb') as file: # b is important -> binary
        fileContent = file.read()
        write_bin(filename_bin, fileContent, 0x1000)

    print(filename_bin,"size:", os.path.getsize(filename_bin))
    
    bin2hex(filename_bin, filename_hex, 0x8000)
    
    print(filename_hex,"size:", os.path.getsize(filename_hex))
    print("************************build done****************************")
except EOFError:
    pass
    
file.close()