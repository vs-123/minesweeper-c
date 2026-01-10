#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "raylib.h"
#include "vector.h"

#define SCREEN_WIDTH 16 * 30
#define SCREEN_HEIGHT 16 * 30 + 50

typedef uint8_t u8;

typedef struct
{
   bool is_mine;
   bool is_revealed;
   bool has_flag;
   u8 adjacent_mines; /* won't ever exceed 9 so... u8 seems good enough */
} cell_t;

DECLARE_VECTOR (cell_t, cell_vector_t)
DECLARE_VECTOR (cell_vector_t, cell_grid_t)

IMPLEMENT_VECTOR (cell_t, cell_vector_t)
IMPLEMENT_VECTOR (cell_vector_t, cell_grid_t)

typedef struct
{
   u8 grid_columns;
   u8 grid_rows;
   u8 cell_size;
   u8 mines_count;
   u8 ui_height;

   cell_grid_t cell_grid;

   float start_time;
   float end_time;
   float pause_duration;
   float confirm_start_time;

   bool has_game_ended;
   bool has_user_won;
   bool is_reset_confirmed;
   bool is_first_click;

   Texture2D mine_texture;
   Texture2D flag_texture;

   Rectangle btn_new_game;
   Rectangle btn_yes;
   Rectangle btn_no;
   Rectangle modal_rect;

   /* colour palette for the numbers */
   Color colour_palette[9];
} mswpr_t;

mswpr_t
mswpr_init (void)
{
   mswpr_t mswpr;

   mswpr.grid_columns = 16;
   mswpr.grid_rows    = 16;
   mswpr.cell_size    = 30;
   mswpr.mines_count  = 40;
   mswpr.ui_height    = 50;

   mswpr.start_time         = GetTime ();
   mswpr.end_time         = 0.0;
   mswpr.pause_duration     = 0.0;
   mswpr.confirm_start_time = 0.0;

   mswpr.has_game_ended     = false;
   mswpr.has_user_won       = false;
   mswpr.is_reset_confirmed = false;
   mswpr.is_first_click     = true;

   mswpr.btn_new_game
       = (Rectangle){ .x = 10, .y = 10, .width = 105, .height = 30 };

   int modal_width   = 300;
   int modal_height  = 150;
   int screen_width  = mswpr.grid_columns * mswpr.cell_size;
   int screen_height = mswpr.grid_rows * mswpr.cell_size + mswpr.ui_height;

   mswpr.modal_rect
       = (Rectangle){ .x      = (float)(screen_width / 2 - modal_width / 2),
                      .y      = (float)(screen_height / 2 - modal_height / 2),
                      .width  = (float)modal_width,
                      .height = (float)modal_height };

   mswpr.btn_yes = (Rectangle){ .x      = mswpr.modal_rect.x + 30,
                                .y      = mswpr.modal_rect.y + modal_height - 50,
                                .width  = 100,
                                .height = 30 };

   mswpr.btn_no = (Rectangle){ .x     = mswpr.modal_rect.x + modal_width - 130,
                               .y     = mswpr.modal_rect.y + modal_height - 50,
                               .width = 100,
                               .height = 30 };

   mswpr.mine_texture = LoadTexture ("assets/mine.png");
   mswpr.flag_texture = LoadTexture ("assets/flag.png");

   Image window_icon = LoadImage ("assets/mine.png");
   SetWindowIcon (window_icon);
   UnloadImage (window_icon);

   /* reset_game(); */

   return mswpr;
}

void
mswpr_free (mswpr_t *mswpr)
{

   if (mswpr->mine_texture.width > 0)
      {
         UnloadTexture (mswpr->mine_texture);
      }
   if (mswpr->flag_texture.width > 0)
      {
         UnloadTexture (mswpr->flag_texture);
      }
}

int
main (void)
{
   InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Minesweeper -- vs-123");
   
   mswpr_t mswpr = mswpr_init ();
   mswpr_free(&mswpr);

   CloseWindow();
   return 0;
}
