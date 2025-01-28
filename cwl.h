#ifndef CWL_H
#define CWL_H

#include <stdint.h>
#include <stdbool.h>

/*  This api is kinda like plutos-fw's main method  */
typedef enum ECallWhy
{
    ECALL_Tick,

    ECALL_KeyUp,
    ECALL_KeyDown,
    ECALL_KeyEnter,

    ECALL_KeyUp_Held,
    ECALL_KeyDown_Held,
    ECALL_KeyEnter_Held,
} ECallWhy;

/* Proccess Method */
void cwl_main(ECallWhy call_why);

/* vm interaction api */
void cwl_init();
void cwl_vm_halt();

/* output method pointers */
typedef struct CWL_OUT
{
    void (*lcd_clear)(void); // clear lcd

    // CALL SAMPLES
    // (0, 2, 10) => [10 : xx xx]
    // (2, 2, 99) => [xx : 99 xx]
    // (4, 2,  1) => [xx : xx 01]
    void (*lcd_output_int)(uint8_t start, uint8_t len, uint16_t value);

    // CALL SAMPLES
    // (0, 2, 'h') => [hx : xx xx]
    // (2, 2, 'o') => [xx : xo xx]
    void (*lcd_output_char)(uint8_t start, char c);

    // CALL SAMPLES
    // (0, 2, 255) => [FF : xx xx]
    // (2, 2, 128) => [xx : 80 xx]
    void (*lcd_output_int_hex)(uint8_t start, uint8_t len, uint16_t value);

    // debugging... may be null, use lvl 3 for info... its appart of the raylib api
    void (*dbg_log)(int lvl, const char *fmt, ...);

} CWL_OUT;

void cwl_set_out(CWL_OUT);

enum cwl_op
{
    Noop,
    Push,
    Pop,
    Add,
    Sub,
    Mut,
    Div,
    Cmp,
    Input,
    Display,
    DisplayChar,
    Jump,
    JumpEq,
    JumpGt,
    JumpLe,
    Halt,
    CWL_OP_MAX
};

typedef struct cwl_vm_inst
{
    uint8_t opp, arg;
} cwl_vm_inst;

typedef union UFlags
{
    struct Flags
    {
        uint8_t prog_running : 1;
        uint8_t cmp_zero : 1;
        uint8_t cmp_neg : 1;
    } f;

    uint8_t v;
} UFlags;

typedef struct cwl_state
{
    enum run_state
    {
        run,
        edit
    } run_state;

    struct cwl_vm_state
    {
        uint8_t sp, ip;
        UFlags flags;
        uint8_t stack[128];
        cwl_vm_inst code[128];
    } vm;

    struct cwl_edit_state
    {
        enum section
        {
            line, // up down effects line var
            opp, // up down effects opp
            arg, // up down effects arg (if op has one)
        } part;
        uint8_t line; // what code[line] is being edited right now
    } ed;

} cwl_state;

#define MAX_STACK (sizeof(cwl.vm.stack) / sizeof(cwl.vm.stack[0]))
#define MAX_CODE (sizeof(cwl.vm.code) / sizeof(cwl.vm.code[0]))



void cwl_debug_loadprog(cwl_vm_inst[128]);
/* debug api */
void cwl_vm_step(); // do one instuction

#endif // CWL_H