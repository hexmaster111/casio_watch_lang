#include "cwl.h"
#include <raylib.h>
#include <math.h>

#define LOGCALL() TraceLog(LOG_INFO, "%s", __func__)

extern cwl_state cwl;

void SimUpdate();

typedef enum Seg
{
    /*     a
     *     _
     *  f |_|  b  <-- g in the center
     *  e |_|  c
     *     d
     * */

    seg_a = 1 << 0,
    seg_b = 1 << 1,
    seg_c = 1 << 2,
    seg_d = 1 << 3,
    seg_e = 1 << 4,
    seg_f = 1 << 5,
    seg_g = 1 << 6,
} Seg;

struct LcdState
{
    // 0 - 5 : main time display, 0,1 hr 2,3min 4,5 sec
    // 6 - 7 : day displ,

    uint8_t d[8];
} lcd_state;

void SetSegment(uint8_t *v, Seg s, int val)
{
    if (val)
        *v |= s;
    else
        *v &= ~s;
}

// a - z ish
void SetSegLetter(uint8_t *v, char ch)
{

    switch (ch)
    {
        // clang-format off
    case 'i': *v = (seg_f | seg_e); break;
    case 'r': *v = (seg_e | seg_g); break;
    case 'x': *v = (seg_e | seg_c); break;
    case 'c': *v = (seg_g | seg_e | seg_d);  break;
    case 'j': *v = (seg_d | seg_c | seg_a); break;
    case 'n': *v = (seg_e | seg_g | seg_c); break;
    case 'u': *v = (seg_e | seg_d | seg_c); break;
    case 'v': *v = (seg_f | seg_b | seg_d); break;
    case 'l': *v = (seg_f | seg_e | seg_d); break;
    case 'f': *v = (seg_a | seg_f | seg_g | seg_e); break;
    case 'h': *v = (seg_f | seg_e | seg_g | seg_c); break;
    case 'm': *v = (seg_e | seg_g | seg_c | seg_a); break;
    case 'o': *v = (seg_e | seg_g | seg_c | seg_d); break;
    case 's': *v = (seg_a | seg_f | seg_c | seg_d); break;
    case 't': *v = (seg_f | seg_g | seg_e | seg_d); break;
    case 'w': *v = (seg_f | seg_b | seg_d | seg_g); break;
    case 'z': *v = (seg_a | seg_b | seg_e | seg_d); break;
    case 'b': *v = (seg_f | seg_e | seg_d | seg_c | seg_g);  break;
    case 'd': *v = (seg_g | seg_e | seg_d | seg_c | seg_b); break;
    case 'e': *v = (seg_a | seg_f | seg_g | seg_e | seg_d); break;
    case 'g': *v = (seg_a | seg_f | seg_e | seg_d | seg_c); break;
    case 'k': *v = (seg_e | seg_c | seg_g | seg_f | seg_a); break;
    case 'p': *v = (seg_f | seg_a | seg_g | seg_b | seg_e); break;
    case 'q': *v = (seg_f | seg_g | seg_b | seg_a | seg_c); break;
    case 'y': *v = (seg_f | seg_g | seg_b | seg_c | seg_d); break;
    case 'a': *v = (seg_a | seg_f | seg_b | seg_g | seg_e | seg_c);  break;
        // clang-format on

    default:
        TraceLog(LOG_INFO, "invalid char:'%c' dec:%d", ch, ch);
        break;
    }
}

// 0 - 9
void Set7Seg_Number(uint8_t *v, uint8_t n)
{
    switch (n)
    {
        // clang-format off
    case 0: *v = (seg_a | seg_b | seg_c | seg_d | seg_e | seg_f); break;
    case 1: *v = (seg_b | seg_c); break;
    case 2: *v = (seg_a | seg_b | seg_d | seg_g | seg_e); break;
    case 3: *v = (seg_a | seg_b | seg_g | seg_c | seg_d); break;
    case 4: *v = (seg_f | seg_g | seg_b | seg_c); break;
    case 5: *v = (seg_a | seg_f | seg_g | seg_c | seg_d); break;
    case 6: *v = (seg_a | seg_f | seg_e | seg_d | seg_c | seg_g); break;
    case 7: *v = (seg_b | seg_c | seg_a); break;
    case 8: *v = (seg_a | seg_b | seg_c | seg_d | seg_e | seg_f | seg_g); break;
    case 9: *v = (seg_a | seg_b | seg_c | seg_d | seg_f | seg_g); break;
    default:break;
        // clang-format on
    }
}

// 0 - F
void Set7Seg_NumberHex(uint8_t *v, uint8_t n)
{
    switch (n)
    {
        // clang-format off
    case 0x0: 
    case 0x1: case 0x2: case 0x3: 
    case 0x4: case 0x5: case 0x6: 
    case 0x7: case 0x8: case 0x9:  
        Set7Seg_Number(v, n);
    break;

    case 0xA: *v = (seg_a | seg_f | seg_b | seg_g | seg_e | seg_c);  break;
    case 0xB: *v = (seg_f | seg_e | seg_d | seg_c | seg_g);  break;
    case 0xC: *v = (seg_g | seg_e | seg_d);  break;
    case 0xD: *v = (seg_g | seg_e | seg_d | seg_c | seg_b); break;
    case 0xE: *v = (seg_a | seg_f | seg_g | seg_e | seg_d); break;
    case 0xF: *v = (seg_a | seg_f | seg_g | seg_e); break;
    default:break;
        // clang-format on
    }
}

void Draw7Seg(float x, float y, float w, float h, uint8_t state)
{
    const Color set = YELLOW;
    const Color unset = ColorLerp(YELLOW, BLACK, 0.75f);
    const float thick = 2.0f;

    Color a = state & seg_a ? set : unset,
          b = state & seg_b ? set : unset,
          c = state & seg_c ? set : unset,
          d = state & seg_d ? set : unset,
          e = state & seg_e ? set : unset,
          f = state & seg_f ? set : unset,
          g = state & seg_g ? set : unset;

    Vector2 p0 = {x, y},
            p1 = {x + w, y},
            p2 = {x + w, y + (h * 0.5f)},
            p3 = {x + w, y + h},
            p4 = {x, y + h},
            p5 = {x, y + (h * 0.5f)};

    DrawLineEx(p0, p1, thick, a);
    DrawLineEx(p1, p2, thick, b);
    DrawLineEx(p2, p3, thick, c);
    DrawLineEx(p3, p4, thick, d);
    DrawLineEx(p4, p5, thick, e);
    DrawLineEx(p5, p0, thick, f);
    DrawLineEx(p5, p2, thick, g);
}

void SimDraw()
{

    Draw7Seg(75 + (15 * 0), 10 + 50, 10, 20, lcd_state.d[0]);
    Draw7Seg(75 + (15 * 1), 10 + 50, 10, 20, lcd_state.d[1]);
    Draw7Seg(75 + (15 * 2), 10 + 50, 10, 20, lcd_state.d[2]);
    Draw7Seg(75 + (15 * 3), 10 + 50, 10, 20, lcd_state.d[3]);
    Draw7Seg(75 + (15 * 4), 15 + 50, 10, 15, lcd_state.d[4]);
    Draw7Seg(75 + (15 * 5), 15 + 50, 10, 15, lcd_state.d[5]);

    Draw7Seg(80 + 0, 45, 5, 10, lcd_state.d[6]);
    Draw7Seg(80 + 8, 45, 5, 10, lcd_state.d[7]);

    DrawText(TextFormat("Run: %d Zero: %d Neg: %d Sp: %d Ip: %d",
                        cwl.vm.flags.f.prog_running,
                        cwl.vm.flags.f.cmp_zero,
                        cwl.vm.flags.f.cmp_neg,
                        cwl.vm.sp,
                        cwl.vm.ip),
             10, GetScreenHeight() - 10, 10, YELLOW);
}

/* thease methods are called by the Casio Watch Lang*/
void Sim_API_lcd_clear()
{
    LOGCALL();
    for (int i = 0; i < sizeof(lcd_state.d) / sizeof(lcd_state.d[0]); i++)
    {
        lcd_state.d[i] = 0;
    }
}

int GetNumberAtPlace(int value, int dig, int rdx)
{
    // Calculate the divisor to isolate the desired digit
    int divisor = pow(rdx, dig);
    // Isolate the digit and return it
    return (value / divisor) % rdx;
}

void Sim_API_lcd_output_int(uint8_t start, uint8_t len, uint16_t value)
{
    TraceLog(LOG_INFO, "%s(start: %d, len: %d, value: %d)", __func__, start, len, value);

    uint8_t *s = &lcd_state.d[start + (len - 1)];

    // 1024  len 4 value = 1024
    //   55  len 4 value = 55
    // 55    len 2 value = 55

    for (int dig = 0; dig < len; dig++, s -= sizeof(uint8_t))
    {
        int atd = GetNumberAtPlace(value, dig, 10);
        Set7Seg_Number(s, atd);
    }
}

void Sim_API_lcd_output_char(uint8_t start, char value)
{
    TraceLog(LOG_INFO, "%s(start: %d, value: %c)", __func__, start, value);

    SetSegLetter(&lcd_state.d[start], value);
}

void Sim_API_lcd_output_int_hex(uint8_t start, uint8_t len, uint16_t value)
{
    TraceLog(LOG_INFO, "%s(start: %d,len: %d,value: %d)", __func__, start, len, value);
    uint8_t *s = &lcd_state.d[start + (len - 1)];

    // 1024  len 4 value = 1024
    //   55  len 4 value = 55
    // 55    len 2 value = 55

    for (int dig = 0; dig < len; dig++, s -= sizeof(uint8_t))
    {
        int atd = GetNumberAtPlace(value, dig, 16);
        Set7Seg_NumberHex(s, atd);
    }
}

/* Driving code for the simulator */
int main(int argc, char *argv[])
{
    CWL_OUT out = {
        .lcd_clear = Sim_API_lcd_clear,
        .lcd_output_int = Sim_API_lcd_output_int,
        .lcd_output_char = Sim_API_lcd_output_char,
        .lcd_output_int_hex = Sim_API_lcd_output_int_hex,
        .dbg_log = TraceLog};

    cwl_set_out(out);
    cwl_init();
    cwl.run_state = edit;

    cwl_debug_loadprog(
        (cwl_vm_inst[128]){
            {Push, 1}, // 5
            {Push, 5}, // 5 5
            {Add},     // 10
            {Display}, // 10
            {Pop},     //
            {Halt}     //
        });

    InitWindow(126 * 2, 80 * 2, "F-91W sim");
    SetTargetFPS(60);

    // char n = 'a';
    // float next = 0.5f;

    while (!WindowShouldClose())
    {
        SimUpdate();

        cwl_main(ECALL_Tick);

        // if (GetTime() > next)
        // {
        //     next = GetTime() + 0.5f;

        //     Sim_API_lcd_output_char(0, n);
        //     Sim_API_lcd_output_int_hex(6, 2, n);

        //     n += 1;

        //     if (n > 'z')
        //     {
        //         n = 'a';
        //     }
        // }

        BeginDrawing();
        ClearBackground(BLACK);

        //\narrow Up/down: edit\nspace: enter\nshift+space: hold enter
        DrawText("S: step\nH: halt", 10, 10, 9, YELLOW);

        SimDraw();
        EndDrawing();
    }

    CloseWindow();
}

void SimUpdate()
{
    if (IsKeyPressed(KEY_UP) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyUp)");
        cwl_main(ECALL_KeyUp);
    }
    else if (IsKeyPressed(KEY_UP) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyUp_Held)");
        cwl_main(ECALL_KeyUp_Held);
    }

    if (IsKeyPressed(KEY_DOWN) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyDown)");
        cwl_main(ECALL_KeyDown);
    }
    else if (IsKeyPressed(KEY_DOWN) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyDown_Held)");
        cwl_main(ECALL_KeyDown_Held);
    }

    if (IsKeyPressed(KEY_SPACE) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyEnter)");
        cwl_main(ECALL_KeyEnter);
    }
    else if (IsKeyPressed(KEY_SPACE) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    {
        TraceLog(LOG_INFO, "cwl_main(ECALL_KeyEnter_Held)");
        cwl_main(ECALL_KeyEnter_Held);
    }

    if (IsKeyPressed(KEY_S))
    {
        cwl_vm_step();
    }

    if (IsKeyPressed(KEY_H))
    {
        cwl_vm_halt();
    }
}