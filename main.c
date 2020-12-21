#include <genesis.h>
#include <gfx.h>
#include <sfx.h>
#include <tiles.h>
#include <sprites.h>
#include <music.h>


#define SFX_LASER 64
#define SFX_EXPLOSION 65

#define MAX_ENEMIES 10
#define MAX_BULLETS 28
#define MAX_PLAYER_BULLETS 3
#define SHOT_INTERVAL 50

u16 shotByPlayer = 0;


#define LEFT_EDGE 0
#define RIGHT_EDGE 320
#define BOTTOM_EDGE 224

#define ANIM_STRAIGHT 0
#define ANIM_MOVE 1

u16 enemiesLeft = 0;
u16 bulletsOnScreen = 0;
u16 shotTicker = 0;

int level = 0;
int offset = 0;

int i = 0;

int j = 0;

int thex = 0; 
int they = 0;
int val = 1;

int player_lives;
int player_x;
int player_y;
int player_velx;
int player_vely;
int player_health;

int enemy1_x;
int enemy1_y;
int enemy1_velx;
int enemy1_vely;
int enemy1_health;

int enemy_max_vel;
int enemy_direction;

int shot_timer;

int level_number;

int score = 0;
char hud_string[40] = "";

Sprite* logo;

static void updateLogo();
static void updateTitle();
static void updateGame();


typedef struct {
	int x;
	int y;
	int w;
	int h;
	int velx;
	int vely;
	int health;
	Sprite* sprite;
	char name[6];
} Entity;


void killEntity(Entity* e){
	e->health = 0;
	SPR_setVisibility(e->sprite,HIDDEN);
}

void reviveEntity(Entity* e){
	e->health = 1;
	SPR_setVisibility(e->sprite,VISIBLE);
}

Entity enemies[MAX_ENEMIES];

void positionEnemies(){
    shotTicker++;
    u16 i = 0;

    for(i = 0; i < MAX_ENEMIES; i++){
        Entity* e = &enemies[i];
           if(e->health > 0){
                e->x += e->velx;

                SPR_setPosition(e->sprite,e->x,e->y);

                /*Shooting*/
               if(shotTicker >= shot_timer){             
                    if( (random() % (10-1+1)+1) > 4 ){
                         shootBullet(*e);
                         shotTicker = 0;
                    }
               }

               if( (e->x+e->w) > RIGHT_EDGE){
                   e->velx = -(enemy_max_vel);
                   //e->y += 16;
               }
              else if(e->x < LEFT_EDGE){
                 e->velx = enemy_max_vel;
                 //e->y += 16;
               }
           }

    }
 
}

Entity player = {0, 0, 16, 16, 0, 0, 0, "PLAYER"};

void positionPlayer(){

    /*Add the player's velocity to its position*/
    player.x += player.velx;

    /*Keep the player within the bounds of the screen*/
    if(player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if(player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    /*Let the Sprite engine position the sprite*/
    SPR_setPosition(player.sprite,player.x,player.y);

}


Entity bullets[MAX_BULLETS];


void positionBullets(){
      u16 i = 0;
      Entity *b;
      for(i = 0; i < MAX_BULLETS; i++){
        b = &bullets[i];
        if(b->health > 0){
           b->y += b->vely;

           if(b->y + b->h < 0){
              killEntity(b);
              bulletsOnScreen--;
              shotByPlayer--;

            } else if(b->y > BOTTOM_EDGE){
              killEntity(b);
              bulletsOnScreen--;
           } else{
              SPR_setPosition(b->sprite,b->x,b->y);
           }
        }
      }
}


void shootBullet(Entity Shooter){
    bool fromPlayer = (Shooter.y > 100);
    if( bulletsOnScreen < MAX_BULLETS ){
       
       if(fromPlayer == TRUE){
            if(shotByPlayer >= MAX_PLAYER_BULLETS){
               return;
             }
        }

        Entity* b;
        u16 i = 0;

        for(i=0; i<MAX_BULLETS; i++){
            b = &bullets[i];
            if(b->health == 0){

                b->x = Shooter.x + 4;
                b->y = Shooter.y;

                reviveEntity(b);

               if(fromPlayer == TRUE){
                         SPR_setVFlip(b->sprite,FALSE);
                         b->vely = -5;
                         shotByPlayer++;
                } else{
                    SPR_setVFlip(b->sprite,TRUE);
                    b->vely = 3;
                }

                SPR_setPosition(b->sprite,b->x,b->y);
                bulletsOnScreen++;

                XGM_startPlayPCM(SFX_LASER,1,SOUND_PCM_CH2);

                break;
            }
        }	
    }
}

void updateScoreDisplay(){
    sprintf(hud_string,"SCORE: %d - LIVES: %d - LEVEL : %d",score, player_lives, level_number);
    VDP_clearText(0,0,40);
    VDP_drawText(hud_string,0,0);
}



static void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		if (state & BUTTON_START)
		{
                           if (level == 0){
                                 // start music
                                 // SND_startPlay_XGM(gametitle);
                                 SND_startPlay_XGM(singlelevel);
                                 SPR_init(0,0,0);
	                     level = 1;

                                 VDP_clearTextLine(10);
                                 VDP_clearTextLine(23);

                                 VDP_loadTileSet(background.tileset,1,DMA);
                                 VDP_setPalette(PAL1, background.palette->data);
                                
                                 for( i=0; i < 1280; i++){
                                    thex = i % 40;
                                    they = i / 40;
                                    val = (random() %  (10-1+1))+1;
                                    if(val > 3) val = 1;
                                    VDP_setTileMapXY(BG_B,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,val), thex, they );
                                 }

                                player_lives = 3;
                                score = 0;
                                enemiesLeft = 0;
                                enemy_max_vel = 1;
                                shot_timer =  SHOT_INTERVAL;
                                enemy_direction = 1;
                                level_number = 1;

                            }
                           else if (level == 1)
                           {
                                 // start music
                                 // SND_startPlay_XGM(singlelevel);

                                 SPR_init(0,0,0);

                                 VDP_setPalette(PAL2, ship.palette->data);

                                 //Add the player
                                 player.x = 152;
                                 player.y = 192;
                                 player.health = 1;
                                 player.sprite = SPR_addSprite(&ship,player.x,player.y,TILE_ATTR(PAL2,0,FALSE,FALSE));
                                 
                                 VDP_setPalette(PAL3, enemie.palette->data);

                                 //Create all enemy sprites
                                 Entity* e = enemies;
                                 
                                 for (j = 1 ; j <= 3 ; j++){ 
                                      for(i = 0; i < MAX_ENEMIES/3; i++){

                                         e->y = j * 32;
                                         e->x = i * 32;

                                         e->w = 16;
                                         e->h = 16;
                                         e->velx = enemy_max_vel * enemy_direction;
                                         e->health = 1;    
                                         e->sprite = SPR_addSprite(&enemie,e->x,e->y,TILE_ATTR(PAL3,0,TRUE,FALSE));
                                         sprintf(e->name, "En%d%d",i,j);
                                         enemiesLeft++;
                                         e++;	
                                      }
                                      enemy_direction = enemy_direction * -1;
                                 }

                                 VDP_setPaletteColor(34,RGB24_TO_VDPCOLOR(0x0078f8));
  
                                 VDP_setPalette(PAL1, bullet.palette->data);

                                  /*Create all bullet sprites*/
                                  Entity* b = bullets;
                                  for(i = 0; i < MAX_BULLETS; i++){
                                     b->x = 0;
                                     b->y = -10;
                                     b->w = 8;
                                     b->h = 8;
                                     b->sprite = SPR_addSprite(&bullet,bullets[0].x,bullets[0].y,TILE_ATTR(PAL1,0,FALSE,FALSE));
                                     sprintf(b->name, "Bu%d",i);
                                     b++;
                                  }

                                 VDP_clearTextLine(10);
                                 VDP_clearTextLine(23);

                                 level = 2;
                                                      
		    }else if (level == 3){

                               VDP_clearTextLine(10);
                               VDP_clearTextLine(23);
                               enemy_max_vel = enemy_max_vel + 1;
                               shot_timer = shot_timer - 3;
                               level_number += 1;

                               level = 1;

                           }else if (level == 4){

                               VDP_clearTextLine(10);
                               VDP_clearTextLine(23);
                               
                               level = 0;
                           }
                    }
	}
                      

            if (level == 2){
                        
                           if (joy == JOY_1){
                              if (state & BUTTON_RIGHT){
                                 player.velx = 2;
                                 SPR_setAnim(player.sprite,ANIM_MOVE);
                                 SPR_setHFlip(player.sprite,TRUE);
                              }
                             else if (state & BUTTON_LEFT)
                              {
                                 player.velx = -2;
                                 SPR_setAnim(player.sprite,ANIM_MOVE);
                                 SPR_setHFlip(player.sprite,FALSE);
                              }
                             else{
                                 if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ) {
                                   player.velx = 0;
                                   SPR_setAnim(player.sprite,ANIM_STRAIGHT);
                                  }
                             }
                         }

                         if (state & BUTTON_B & changed)
                         {
                             shootBullet(player);
                         }

                        updateScoreDisplay();

            }
}


int collideEntities(Entity* a, Entity* b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}


void handleCollisions(){
      Entity* b;
      Entity* e;
      int i = 0;
      int j = 0;
      for(i = 0; i < MAX_BULLETS; i++){
          b = &bullets[i];
          if(b->health > 0){
             if(b->vely < 0){ //This is new!
                 for (j = 0; j < MAX_ENEMIES; j++)
                 {
                        e = &enemies[j];

                        if(e->health > 0){

                              if(collideEntities( b, e )){

                                   killEntity(e);
                                   killEntity(b);
 
                                   enemiesLeft--;
                                   bulletsOnScreen--;
                                   shotByPlayer--;

                                   XGM_startPlayPCM(SFX_EXPLOSION,1,SOUND_PCM_CH2);

                                   score += 10;
                                   updateScoreDisplay();
                          
                                  break;
                             } 
                       }
                 }
             } else{ //Also this

                     if(collideEntities(b,&player)){
                           killEntity(b);
                           XGM_startPlayPCM(SFX_EXPLOSION,1,SOUND_PCM_CH2);

                           if (player_lives > 0){
                              player_lives -= 1;
                              player.x = 160;
                           }else{
                               killEntity(&player);
                               VDP_drawText("GAME OVER", 15, 10);
                               // start music
                               SND_startPlay_XGM(gameover);
                               level = 4;
                           }
                     }
             } 
          }
      }
}



int main()
{
            JOY_init();
            JOY_setEventHandler( myJoyHandler );

            XGM_setPCM(SFX_EXPLOSION, sfx_explosion, sizeof(sfx_explosion));           

            XGM_setPCM(SFX_LASER, sfx_laser, sizeof(sfx_laser));

	while(1)
	{
                       
                       if (level == 0){
                           updateLogo();
                        }

                        if (level == 1){
                           updateTitle();
                        }

                        if (level == 2){
                           updateGame();
                        }


                        SPR_update();

                        VDP_waitVInt();

		VDP_waitVSync();

                        SYS_disableInts();
                        
                        //Code that loads the tile, draws the background...
                        SYS_enableInts();

	}
	return (0);
}


static void updateLogo()
{ 
            SPR_init();
            //-------------------------------------------------------------    
            //Sprite* SPR_addSprite(const SpriteDefinition * 	spriteDef, s16 x, s16 y, u16 attribute)
            //-------------------------------------------------------------    
             //spriteDef: The address in memory of our compiled sprite resource. This is the name we gave our asset in the resource file. Don't forget the & operator!
             //x: The x-coordinate (in pixels!) of our sprite

             //y: The y-coordinate (in pixels!) of our sprite
             //attribute: These are the same as the attributes for tiles
            //-------------------------------------------------------------    
            logo = SPR_addSprite(&genezlogofull,90,35,TILE_ATTR(PAL3,0, FALSE, FALSE));
            //-------------------------------------------------------------   
	
            VDP_drawText("Gene-Z Games 1998 - 2020", 7, 23);
}

static void updateTitle()
{

            
           // VDP_drawText("STARSHIP GAME", 12, 10);
            VDP_drawText("SPACE BLAST PROCESSING", 8, 10);

            VDP_drawText("PRESSIONE START PARA INICIAR", 5, 23);

}

static void updateGame()
{
           VDP_setVerticalScroll(BG_B,offset -= 5);
           if(offset >= 256) offset = 0;
           
           VDP_setScrollingMode(HSCROLL_PLANE,VSCROLL_PLANE);       
           
           positionPlayer();
           positionBullets();
           positionEnemies();
           handleCollisions();

           if (enemiesLeft == 0){
                           VDP_drawText("PRESS START TO NEXT LEVEL", 7, 10);
                                                
                           // start music
                           // SND_startPlay_XGM(congratulations);
                           level = 3;
           }
}



