#include "cwl.h"
#include <raylib.h>
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
    uint8_t d0, d1, d2, d3, d4, d5, d6;
} lcd_state;

void SetSegment(uint8_t *v, Seg s, int val)
{
    if (val)
        *v |= s;
    else
        *v &= ~s;
}

// a - z ish
void SetSegLetter(uint8_t *v, char c)
{
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
    const int sz = 10;

    Draw7Seg(100 + (15 * 0), 10, 10, 20, lcd_state.d0);
    Draw7Seg(100 + (15 * 1), 10, 10, 20, lcd_state.d2);
    Draw7Seg(100 + (15 * 2), 10, 10, 20, lcd_state.d3);
    Draw7Seg(100 + (15 * 3), 10, 10, 20, lcd_state.d4);

    Draw7Seg(100 + (15 * 4), 15, 10, 15, lcd_state.d5);
    Draw7Seg(100 + (15 * 5), 15, 10, 15, lcd_state.d6);

    DrawText(TextFormat("Run: %d Zero: %d Neg: %d Sp: %d Ip: %d",
                        cwl.vm.flags.f.prog_running,
                        cwl.vm.flags.f.cmp_zero,
                        cwl.vm.flags.f.cmp_neg,
                        cwl.vm.sp,
                        cwl.vm.ip),
             sz, GetScreenHeight() - sz, sz, YELLOW);
}

/* thease methods are called by the Casio Watch Lang*/
void Sim_API_lcd_clear() { LOGCALL(); }
void Sim_API_lcd_output_int(uint8_t start, uint8_t len, uint16_t value) { LOGCALL(); }

void Sim_API_lcd_output_str(uint8_t start, char value)
{
    TraceLog(LOG_INFO, "%s(%d, %c)", __func__, start, value);
}

void Sim_API_lcd_output_int_hex(uint8_t start, uint8_t len, uint16_t value)
{
    TraceLog(LOG_INFO, "%s(%d, %d, %d)", __func__, start, len, value);
}

/* Driving code for the simulator */
int main(int argc, char *argv[])
{
    lcd_state.d0 = 0;

    // SetSegment(&lcd_state.d0, (seg_a | seg_b | seg_c | seg_d | seg_e | seg_f), 1);

    CWL_OUT out = {
        .lcd_clear = Sim_API_lcd_clear,
        .lcd_output_int = Sim_API_lcd_output_int,
        .lcd_output_str = Sim_API_lcd_output_str,
        .lcd_output_int_hex = Sim_API_lcd_output_int_hex,
        .dbg_log = TraceLog};

    cwl_set_out(out);
    cwl_init();

    // cwl_debug_loadprog(
    //     (cwl_vm_inst[128]){
    //         {Push, 5}, // 5
    //         {Push, 5}, // 5 5
    //         {Add},     // 10
    //         {Display}, // 10
    //         {Pop},     //
    //     });

    cwl_debug_loadprog(
        (cwl_vm_inst[128]){
            /*0*/ {Push, 5}, // 5
            /*1*/ {Push, 4}, // 4 5
            /*2*/ {Cmp},
            /*3*/ {JumpEq, 9},
            /*4*/ {DisplayChar, 'D'},
            /*5*/ {0xFF},
            /*6*/ {},
            /*7*/ {},
            /*8*/ {},
            /*9*/ {DisplayChar, 'S'},
            /*A*/ {0xFF},
            /*B*/ {},
            /*C*/ {},
            /*D*/ {},
        });

    float anm_time = 0;
    int now = 0;

    InitWindow(126 * 2, 80 * 2, "F-91W sim");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        SimUpdate();

        if (GetTime() > anm_time)
        {
            anm_time = GetTime() + 0.5f;
            now += 1;
            if (now > 15)
                now = 0;

            Set7Seg_NumberHex(&lcd_state.d0, now);
        }

        cwl_main(ECALL_Tick);

        BeginDrawing();
        ClearBackground(BLACK);

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