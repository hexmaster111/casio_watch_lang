# vm thoughts

4 bits encode opperation

## opcode table

| num  | opp | arg  | desc                                                              |
| :--- | --- | ---- | ----------------------------------------------------------------- |
| 0    | nop |      | dose nothing but burn one cycle                                   |
| 1    | psh | val  | push <val> onto the top of the stack                              |
| 2    | pop |      | removes one val from the top of the stack                         |
| 3    | add |      | adds two values on top of stack and stores on stack               |
| 4    | sub |      | subtracts two values on top of the stack and stores on the stack  |
| 5    | mut |      | mutliplies two values on top of the stack and stores on the stack |
| 6    | div |      | devides two values on top of the stack and stores on the stack    |
| 7    | cmp |      | compares two values on top of the stack and sets jump flags       |
| 8    | inp |      | asks user for input, stores value on top of stack                 |
| 9    | dsp |      | displays value on top of stack, DOSE NOT POP                      |
| A    | dsc | char | displays the char arg                                             |
| B    | jmp | line | sets IP to <line>                                                 |
| C    | jeq | line | sets IP to <line> IF cmp_zero flag is unset                       |
| D    | jgt | line | sets IP to <line> IF cmp_neg flag is unset                        |
| E    | jle | line | sets IP to <line> IF cmp_neg flag is set                          |
