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

/* cwl edit api */
void cwl_ed_draw_state();


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
    if (cwl.run_state == edit)
    {
        cwl_edit(call_why);
    }
}

void cwl_edit(ECallWhy s)
{
    if (s == ECALL_Tick)
    {
        return;
    }

    switch (cwl.ed.part)
    {
    case opp:
        break;

    case arg:
        break;

    case line:
        break;

    default:
        break;
    }


    cwl_ed_draw_state();
}




void cwl_ed_draw_state()
{
    
}



/* virtual machine parts */

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
