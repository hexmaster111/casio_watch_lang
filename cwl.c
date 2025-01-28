#include "cwl.h"

/* Output api */
static CWL_OUT out;
cwl_state cwl;

#define LOG(MSG)     \
    if (out.dbg_log) \
    out.dbg_log(3, (MSG))

/* cwl vm api */
void cwl_vm_step(); // do one instuction
void cwl_vm_halt(); // stop the vm
void cwl_run(ECallWhy);

/* cwl edit api */
void cwl_ed_draw_state();

void cwl_edit(ECallWhy s);

/* opcode querys */
const char *cwl_ed_get_opcode_lcd_chars(enum cwl_op opcode);
int cwl_opcode_uses_arg(enum cwl_op opcode);

void cwl_init()
{
    cwl.vm.ip = cwl.vm.sp = 0;
    cwl.vm.flags.v = 0;

    for (int i = 0; i < 128; i++)
    {
        cwl.vm.stack[i] = cwl.vm.code[i].arg = cwl.vm.code[i].opp = 0;
    }
}

/* input api */
void cwl_main(ECallWhy call_why)
{
    if (call_why == ECALL_KeyEnter_Held) /* hold enter for run / edit mode, resets the VM */
    {
        cwl.vm.ip = cwl.vm.sp = 0;
        cwl.vm.flags.v = 0;
        out.lcd_clear();

        /* edit to run state transition*/
        if (cwl.run_state == edit)
        {
            cwl.run_state = run;
            cwl.vm.flags.f.prog_running = 1;
        }
        /* run to edit state transition*/
        else if (cwl.run_state == run)
        {
            cwl.run_state = edit;
        }
    }

    if (cwl.run_state == edit)
    {
        cwl_edit(call_why);
    }
    else if (cwl.run_state == run)
    {
        cwl_run(call_why);
    }
}

/* editor */

void cwl_edit(ECallWhy s)
{
    if (s == ECALL_Tick)
    {
        return;
    }

    switch (cwl.ed.part)
    {
    case line:
        if (s == ECALL_KeyEnter)
        {
            cwl.ed.part = opp;
        }

        if (s == ECALL_KeyDown)
        {
            cwl.ed.line += 1;
        }

        if (s == ECALL_KeyUp)
        {
            cwl.ed.line -= 1;
        }

        break;

    case opp:
        if (s == ECALL_KeyEnter)
        {
            if (cwl_opcode_uses_arg(cwl.vm.code[cwl.ed.line].opp))
            {
                cwl.ed.part = arg;
            }
            else
            {
                cwl.ed.part = line;
            }
        }

        if (s == ECALL_KeyDown)
        {
            cwl.vm.code[cwl.ed.line].opp -= 1;
        }

        if (s == ECALL_KeyUp)
        {
            cwl.vm.code[cwl.ed.line].opp += 1;
        }

        if (cwl.vm.code[cwl.ed.line].opp > CWL_OP_MAX)
        {
            cwl.vm.code[cwl.ed.line].opp = 0;
        }

        break;

    case arg:
        if (s == ECALL_KeyEnter)
        {
            cwl.ed.part = line;
        }

        if (s == ECALL_KeyDown)
        {
            cwl.vm.code[cwl.ed.line].arg -= 1;
        }

        if (s == ECALL_KeyUp)
        {
            cwl.vm.code[cwl.ed.line].arg += 1;
        }

        break;

    default:
        break;
    }

    cwl_ed_draw_state();
}

void cwl_ed_draw_state()
{
    // 6 - 7 : mode
    out.lcd_clear();

    /* draw the current editor mode*/
    if (cwl.ed.part == line)
    {
        out.lcd_output_char(6, 'l');
        out.lcd_output_char(7, 'n');
    }
    else if (cwl.ed.part == arg)
    {
        out.lcd_output_char(6, 'a');
        out.lcd_output_char(7, 'g');
    }
    else if (cwl.ed.part == opp)
    {
        out.lcd_output_char(6, 'o');
        out.lcd_output_char(7, 'p');
    }

    /* editor line number */
    out.lcd_output_int_hex(0, 2, cwl.ed.line);

    /*opp code display*/
    const char *op_chars = cwl_ed_get_opcode_lcd_chars(cwl.vm.code[cwl.ed.line].opp);
    out.lcd_output_char(2, op_chars[0]);
    out.lcd_output_char(3, op_chars[1]);

    /* argument display */
    if (cwl_opcode_uses_arg(cwl.vm.code[cwl.ed.line].opp))
    {
        out.lcd_output_int_hex(4, 2, cwl.vm.code[cwl.ed.line].arg);
    }
}

/* virtual machine parts */

void cwl_run(ECallWhy w)
{
    /* todo : user input stuff */

    if (w == ECALL_Tick)
    {
        cwl_vm_step();
    }
}

void cwl_vm_push(uint8_t val)
{
    cwl.vm.stack[cwl.vm.sp] = val;
    cwl.vm.sp += 1;

    if (cwl.vm.sp > MAX_STACK)
    {
        LOG("Stack Overflow");
        cwl_vm_halt();
    }
}

uint8_t cwl_vm_peekn(uint8_t n)
{
    if (0 >= cwl.vm.sp - n)
    {
        LOG("Stack Underflow");
        cwl_vm_halt();
    }

    return cwl.vm.stack[cwl.vm.sp - n];
}

uint8_t cwl_vm_pop()
{
    if (0 >= cwl.vm.sp)
    {
        LOG("Stack Underflow");
        cwl_vm_halt();
    }

    cwl.vm.sp -= 1;
    return cwl.vm.stack[cwl.vm.sp];
}

uint8_t cwl_vm_peek()
{

    if (0 >= cwl.vm.sp)
    {
        LOG("Stack Underflow");
        cwl_vm_halt();
    }

    return cwl.vm.stack[cwl.vm.sp - 1];
}

/* evalueates the vm 1 step */
void cwl_vm_step()
{
    if (!cwl.vm.flags.f.prog_running)
    {
        return;
    }

    LOG("STEP");
    cwl_vm_inst c = cwl.vm.code[cwl.vm.ip];
    uint8_t a = 0, b = 0;

    switch (c.opp)
    {
        // clang-format off
    case Noop: /*do nothing*/  break;
    case Push: cwl_vm_push(c.arg); break;
    case Pop: cwl_vm_pop(); break;
        // clang-format on
    case Add:
        a = cwl_vm_pop();
        b = cwl_vm_pop();
        cwl_vm_push(a + b);
        break;
    case Sub:
        a = cwl_vm_pop();
        b = cwl_vm_pop();
        cwl_vm_push(a - b);
        break;
    case Mut:
        a = cwl_vm_pop();
        b = cwl_vm_pop();
        cwl_vm_push(a * b);
        break;
    case Div:
        a = cwl_vm_pop();
        b = cwl_vm_pop();
        cwl_vm_push(a / b);
        break;
    case Cmp:
        a = cwl_vm_peekn(0);
        b = cwl_vm_peekn(1);

        cwl.vm.flags.f.cmp_zero = a == b;
        cwl.vm.flags.f.cmp_neg = 0 > (a - b);
        break;
    case Input:
        break;

    case DisplayChar:
        out.lcd_output_char(0, (char)c.arg);
        break;

    case Display:
        out.lcd_output_int_hex(2, 2, cwl_vm_peek());
        break;

        /* jumps are a little special, they dont set the IP up one IF they jump */
    case Jump:
        cwl.vm.ip = c.arg;
        return;
    case JumpEq:
        if (cwl.vm.flags.f.cmp_zero)
            cwl.vm.ip = c.arg;
        else
            goto NO_JUMP;
        return;

    case JumpLe:
        if (cwl.vm.flags.f.cmp_neg)
            cwl.vm.ip = c.arg;
        else
            goto NO_JUMP;
        return;

    default:
        cwl_vm_halt();
        return;
    }

NO_JUMP:

    cwl.vm.ip += 1;
}

void cwl_debug_loadprog(cwl_vm_inst code[128])
{
    for (int i = 0; i < 128; i++)
    {
        cwl.vm.code[i] = code[i];
    }

    cwl.vm.flags.f.prog_running = 1;
    LOG("Debug Loaded Program");
}

void cwl_vm_halt()
{
    cwl.vm.flags.f.prog_running = 0;
    LOG("VM HALTED");
}

void cwl_set_out(CWL_OUT upo) { out = upo; }

const char *cwl_ed_get_opcode_lcd_chars(enum cwl_op opcode)
{
    switch (opcode)
    {
        // clang-format off
    case Noop:        return "no";
    case Push:        return "pu";
    case Pop:         return "po";
    case Add:         return "ad";
    case Sub:         return "su";
    case Div:         return "di";
    case Cmp:         return "cp";
    case Input:       return "in";
    case Display:     return "ds";
    case DisplayChar: return "dc";
    case Jump:        return "ju";
    case JumpEq:      return "je";
    case JumpGt:      return "jg";
    case JumpLe:      return "jl";
    case Halt:        return "hl";
        // clang-format on

    default:
        if (out.dbg_log)
            out.dbg_log(3, "%d %s %s(%d) default", __LINE__, __FILE__, __func__, opcode);

        return "er"; // error
        break;
    }
}

int cwl_opcode_uses_arg(enum cwl_op opcode)
{
    switch (opcode)
    {
        // clang-format off
    case Noop:        return 0;
    case Push:        return 1;
    case Pop:         return 0;
    case Add:         return 0;
    case Sub:         return 0;
    case Div:         return 0;
    case Cmp:         return 0;
    case Input:       return 0;
    case Display:     return 0;
    case DisplayChar: return 1;
    case Jump:        return 1;
    case JumpEq:      return 1;
    case JumpGt:      return 1;
    case JumpLe:      return 1;
    case Halt:        return 0;
        // clang-format on

    default:
        if (out.dbg_log)
            out.dbg_log(3, "%d %s %s(%d) default", __LINE__, __FILE__, __func__, opcode);

        return 0; // error
        break;
    }
}