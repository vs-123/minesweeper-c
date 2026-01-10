#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "vector.h"

#define NOT_IMPL assert (0 && "NOT IMPLEMENTED");

#define SCREEN_WIDTH 16 * 30
#define SCREEN_HEIGHT 16 * 30 + 50

typedef unsigned int ui;
typedef uint8_t u8;

typedef struct
{
   bool is_mine;
   bool is_revealed;
   bool has_flag;
   u8 adjacent_mines_count; /* won't ever exceed 9 so... u8 seems good enough
                             */
} cell_t;

DECLARE_VECTOR (cell_t, cell_row_t)
DECLARE_VECTOR (cell_row_t, cell_grid_t)

IMPLEMENT_VECTOR (cell_t, cell_row_t)
IMPLEMENT_VECTOR (cell_row_t, cell_grid_t)

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

mswpr_t mswpr_init (void);
void mswpr_free (mswpr_t *mswpr);
void mswpr_place_mines (mswpr_t *mswpr);
void mswpr_calc_adj_mines (mswpr_t *mswpr);
void mswpr_reset (mswpr_t *mswpr);
void mswpr_place_mines (mswpr_t *mswpr);
void mswpr_calc_adj_mines (mswpr_t *mswpr);
void mswpr_update (mswpr_t *mswpr);
void mswpr_draw (mswpr_t *mswpr);
void mswpr_run (mswpr_t *mswpr);

int
main (void)
{
   InitWindow (SCREEN_WIDTH, SCREEN_HEIGHT, "Minesweeper -- vs-123");
   mswpr_t mswpr = mswpr_init ();

   mswpr_run (&mswpr);

   mswpr_free (&mswpr);
   CloseWindow ();

   return 0;
}

mswpr_t
mswpr_init (void)
{
   mswpr_t mswpr;

   mswpr.grid_columns = 16;
   mswpr.grid_rows    = 16;
   mswpr.cell_size    = 30;
   mswpr.mines_count  = 40;
   mswpr.ui_height    = 50;

   mswpr.cell_grid = cell_grid_t_new (mswpr.grid_rows);
   cell_grid_t_resize (&mswpr.cell_grid, mswpr.grid_rows);

   for (ui i = 0; i < mswpr.grid_rows; i++)
      {
         cell_row_t row = cell_row_t_new (mswpr.grid_columns);
         cell_row_t_resize (&row, mswpr.grid_columns);
         *cell_grid_t_at (&mswpr.cell_grid, i) = row;
      }

   mswpr.start_time         = GetTime ();
   mswpr.end_time           = 0.0;
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

   mswpr.btn_yes = (Rectangle){ .x = mswpr.modal_rect.x + 30,
                                .y = mswpr.modal_rect.y + modal_height - 50,
                                .width  = 100,
                                .height = 30 };

   mswpr.btn_no = (Rectangle){ .x     = mswpr.modal_rect.x + modal_width - 130,
                               .y     = mswpr.modal_rect.y + modal_height - 50,
                               .width = 100,
                               .height = 30 };

   mswpr.mine_texture = LoadTexture ("assets/mine.png");
   mswpr.flag_texture = LoadTexture ("assets/flag.png");

   mswpr.colour_palette[0] = WHITE;
   mswpr.colour_palette[1] = (Color){ 0, 70, 241, 255 };
   mswpr.colour_palette[2] = DARKGREEN;
   mswpr.colour_palette[3] = RED;
   mswpr.colour_palette[4] = DARKBLUE;
   mswpr.colour_palette[5] = ORANGE;
   mswpr.colour_palette[6] = (Color){ 64, 224, 208, 255 };
   mswpr.colour_palette[7] = BLACK;
   mswpr.colour_palette[8] = GRAY; // 8

   Image window_icon = LoadImage ("assets/mine.png");
   SetWindowIcon (window_icon);
   UnloadImage (window_icon);

   /* reset_game(); */
   mswpr_reset (&mswpr);

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

void
mswpr_reset (mswpr_t *mswpr)
{
   for (ui i = 0; i < mswpr->grid_rows; i++)
      {
         cell_row_t_free (cell_grid_t_at (&mswpr->cell_grid, i));
      }
   cell_grid_t_free (&mswpr->cell_grid);

   mswpr->cell_grid = cell_grid_t_new (mswpr->grid_rows);
   cell_grid_t_resize (&mswpr->cell_grid, mswpr->grid_rows);

   for (ui i = 0; i < mswpr->grid_rows; i++)
      {
         cell_row_t row = cell_row_t_new (mswpr->grid_columns);
         cell_row_t_resize (&row, mswpr->grid_columns);
         *cell_grid_t_at (&mswpr->cell_grid, i) = row;
      }

   mswpr_place_mines (mswpr);
   mswpr_calc_adj_mines (mswpr);

   mswpr->has_game_ended = false;
   mswpr->has_user_won   = false;
   mswpr->start_time     = GetTime ();
   mswpr->end_time       = 0.0;
   mswpr->pause_duration = 0.0;
   mswpr->is_first_click = true;
}

void
mswpr_place_mines (mswpr_t *mswpr)
{
   ui mines_placed = 0;

   while (mines_placed < mswpr->mines_count)
      {
         int row = rand () % mswpr->grid_rows;
         int col = rand () % mswpr->grid_columns;

         cell_row_t *current_cell_row
             = cell_grid_t_at (&mswpr->cell_grid, row);
         cell_t *current_cell = cell_row_t_at (current_cell_row, col);

         if (!current_cell->is_mine)
            {
               current_cell->is_mine = true;
               mines_placed++;
            }
      }
}

void
mswpr_calc_adj_mines (mswpr_t *mswpr)
{
   for (ui row = 0; row < mswpr->grid_rows; row++)
      {
         for (ui col = 0; col < mswpr->grid_columns; col++)
            {
               cell_row_t *current_cell_row
                   = cell_grid_t_at (&mswpr->cell_grid, row);
               cell_t *current_cell = cell_row_t_at (current_cell_row, col);

               if (current_cell->is_mine)
                  {
                     continue;
                  }

               ui count = 0;

               for (int i = -1; i <= 1; i++)
                  {
                     for (int j = -1; j <= 1; j++)
                        {
                           int new_row = row + i;
                           int new_col = col + j;

                           if (new_row >= 0 && new_row < mswpr->grid_rows
                               && new_col >= 0
                               && new_col < mswpr->grid_columns)
                              {
                                 if (current_cell->is_mine)
                                    {
                                       count++;
                                    }
                              }
                        }
                  }

               current_cell->adjacent_mines_count = count;
            }
      }
}

void
mswpr_update (mswpr_t *mswpr)
{
   NOT_IMPL;
}

void
mswpr_draw (mswpr_t *mswpr)
{
   NOT_IMPL;
}

void
mswpr_run (mswpr_t *mswpr)
{
   while (!WindowShouldClose ())
      {
         mswpr_update (mswpr);
         mswpr_draw (mswpr);
      }
}
