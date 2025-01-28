# vm thoughts

4 bits encode opperation

## opcode table

| num  | lcd | opp | arg  | desc                                                                     |
| :--- | --- | --- | ---- | ------------------------------------------------------------------------ |
| 0    | no  | nop |      | dose nothing but burn one cycle                                          |
| 1    | pu  | psh | val  | push <val> onto the top of the stack                                     |
| 2    | po  | pop |      | removes one val from the top of the stack                                |
| 3    | ad  | add |      | adds two values on top of stack and stores on stack                      |
| 4    | su  | sub |      | subtracts two values on top of the stack and stores on the stack         |
| 5    | mt  | mut |      | mutliplies two values on top of the stack and stores on the stack        |
| 6    | dv  | div |      | devides two values on top of the stack and stores on the stack           |
| 7    | cm  | cmp |      | compares two values on top of the stack and sets jump flags DOSE NOT POP |
| 8    | in  | inp |      | asks user for input, stores value on top of stack                        |
| 9    | ds  | dsp |      | displays value on top of stack, DOSE NOT POP                             |
| A    | dc  | dsc | char | displays the char arg                                                    |
| B    | jm  | jmp | line | sets IP to <line>                                                        |
| C    | je  | jeq | line | sets IP to <line> IF cmp_zero flag is unset                              |
| D    | jg  | jgt | line | sets IP to <line> IF cmp_neg flag is unset                               |
| E    | gl  | jle | line | sets IP to <line> IF cmp_neg flag is set                                 |
| F    | hl  | hlt |      | sets Run flag to 0                                                       |
