rm code

####################
# Output a character 
####################
# Subtract reg_a from reg_a (to zero it) then add 'H'
echo -e -n "\x15\x04H" >> code
# STORE reg_a in 0x000 (the Serial port)
echo -e -n "\xD0\x00" >> code

echo -e -n "\x15\x04e\xD0\x00" >> code
echo -e -n "\x15\x04l\xD0\x00" >> code
echo -e -n "\x15\x04l\xD0\x00" >> code
echo -e -n "\x15\x04o\xD0\x00" >> code
echo -e -n "\x15\x04 \xD0\x00" >> code
echo -e -n "\x15\x04W\xD0\x00" >> code
echo -e -n "\x15\x04o\xD0\x00" >> code
echo -e -n "\x15\x04r\xD0\x00" >> code
echo -e -n "\x15\x04l\xD0\x00" >> code
echo -e -n "\x15\x04d\xD0\x00" >> code
echo -e -n "\x15\x04\n\xD0\x00" >> code


## 00 is the halt instruction
echo -e -n "\x00" >> code

od -tx1 code
