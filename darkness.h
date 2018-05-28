#ifndef DARKEST_H
#define DARKEST_H

#define CP_RED              1
#define CP_GREEN            2
#define CP_YELLOW           3
#define CP_BLUE             4
#define CP_MAGENTA          5
#define CP_CYAN             6
#define CP_WHITE            7
#define CP_DIALOG           15

#define ANSWER_NO           0
#define ANSWER_YES          1
#define ANSWER_NODEFAULT    -1

#define SIDE_PLAYER         0
#define SIDE_ENEMY          1
#define PARTY_SIZE          4
#define MAX_ROSTER_SIZE     8
#define MAX_CLASS_ATTACKS   8
#define MAX_CHAR_NAME_LEN   32
#define INVENTORY_SIZE      10

#define BRIEF_FLOORS        1
#define SHORT_FLOORS        3
#define MEDIUM_FLOORS       4
#define LONG_FLOORS         5
#define EXHAUSTING_FLOORS   7

#define MAP_IN_VIEW         0x01
#define MAP_WAS_SEEN        0x02

#define ATK_SELF            0
#define ATK_MELEE           1
#define ATK_RANGED          2

#define STAT_HP             -1
#define STAT_MAXHP          0
#define STAT_ACCURACY       1
#define STAT_DODGE          2
#define STAT_DAMAGE_MIN     3
#define STAT_DAMAGE_MAX     4
#define STAT_PROTECTION     5
#define STAT_SPEED          6
#define STAT_CRITICAL       7
#define STAT_COUNT          8

#define DIR_NORTH           0
#define DIR_NORTHEAST       1
#define DIR_EAST            2
#define DIR_SOUTHEAST       3
#define DIR_SOUTH           4
#define DIR_SOUTHWEST       5
#define DIR_WEST            6
#define DIR_NORTHWEST       7
#define DIR_NONE            8
#define DIR_COUNT           9

#define TILE_SOLID          0x01
#define TILE_OPAQUE         0x02

#define DUNGEON_COUNT       5

#define SIZE_TINY           0
#define SIZE_SMALL          1
#define SIZE_AVERAGE        2
#define SIZE_LARGE          3
#define SIZE_HUGE           4
#define SIZE_EXCESSIVE      5
#define SIZE_COUNT          6

#define GOAL_EXPLORE        0
#define GOAL_KILLALL        1
#define GOAL_ACTIVATE       2
#define GOAL_COLLECT        3
#define GOAL_BOSS           4
#define GOAL_COUNT          5

#define SEX_MALE            0
#define SEX_FEMALE          1
#define SEX_NEUTER          2

#define ITEM_ARMOUR         0
#define ITEM_WEAPON         1
#define ITEM_TRINKET        2
#define ITEM_JUNK           3
#define ITEM_CONSUME        4

#define SLOT_ARMOUR         0
#define SLOT_WEAPON         1
#define SLOT_TRINKET        2

#define ACTION_STEP_NORTH       0
#define ACTION_STEP_NORTHEAST   1
#define ACTION_STEP_EAST        2
#define ACTION_STEP_SOUTHEAST   3
#define ACTION_STEP_SOUTH       4
#define ACTION_STEP_SOUTHWEST   5
#define ACTION_STEP_WEST        6
#define ACTION_STEP_NORTHWEST   7
#define ACTION_WAIT             8
#define ACTION_BASIC_ATTACK     9
#define TICK_MOVE               10
#define TICK_ATTACK             15

struct message_def {
    char *msg;
    int was_shown;

    struct message_def *next;
};

struct attack_def {
    const char *name;
    int target_type;
    int stat_mods[STAT_COUNT];
    const char *description;
};

struct class_def {
    int ident;
    const char *name;
    int glyph;
    int color;
    const struct attack_def *attacks[MAX_CLASS_ATTACKS];
    int base_stats[STAT_COUNT];
    const char *description;
};

struct tile_type {
    int glyph;
    int color;
    int attribute;
    const char *name;
    int flags;
};

struct actor_def {
    int x;
    int y;
    int side;

    struct item_def *inventory[INVENTORY_SIZE];

    char name[MAX_CHAR_NAME_LEN];
    int sex;
    const struct class_def *my_class;
    int class_ident;

    int hp;
    int stress;
    int tick;
};

struct item_type {
    int ident;
    const char *name;
    int max_stack;
    int usage;
    const char *description;
};

struct item_def {
    int qty;
    int type_id;

    const struct item_type *my_type;
};

struct room_def {
    int x1, y1, x2, y2;
    int type;

    int explored;
    struct room_def *next;
};

struct map_def {
    int width;
    int height;
    int *tiles;
    int *visibility;
    struct actor_def **actors;
    struct actor_def *player;

    int turn_number;
    struct room_def *rooms;
    struct message_def *log;
};

struct dungeon_def {
    int size;
    int goal;
    int complete;

    struct actor_def *player;
};

void message_add(struct map_def *map, const char *text);
void message_format(struct map_def *map, const char *text, ...);
void message_freeall(struct map_def *map);


const struct class_def* class_get(int ident);
void actor_generic_ai(struct map_def *map, struct actor_def *actor);
struct actor_def* actor_new(int class_ident);
void actor_destroy(struct actor_def *actor);
int actor_action_step(struct map_def *map, struct actor_def *actor, int direction);
int actor_get_stat(const struct actor_def *actor, int stat_number);
void actor_die(struct map_def *map, struct actor_def *actor);
void actor_take_damage(struct map_def *map, struct actor_def *actor, int amount);
int actor_do_attack(struct map_def *map, struct actor_def *attacker, struct actor_def *victim, struct attack_def *attack);
int actor_add_item(struct actor_def *actor, struct item_def *new_item);
int actor_item_qty(struct actor_def *actor, int item_type);
int actor_remove_items(struct actor_def *actor, int item_type, int to_remove);

const struct item_type* itemtype_get(int ident);
struct item_def* item_new(int type_ident, int qty);
void item_destroy(struct item_def *item);

const struct tile_type* tile_get_info(int tt);
void shift_point(int *x, int *y, int direction);
int point_next_to(int x, int y, int to_x, int to_y);
int direction_between(int x1, int y1, int x2, int y2);

struct map_def* map_create(int width, int height);
void map_destroy(struct map_def *map);
int map_valid_coord(const struct map_def *map, int x, int y);
int map_get_tile(const struct map_def *map, int x, int y);
void map_set_tile(const struct map_def *map, int x, int y, int new_tile);
struct actor_def* map_get_actor(const struct map_def *map, int x, int y);
int map_set_actor(struct map_def *map, int x, int y, struct actor_def *actor);
int map_shift_actor(struct map_def *map, struct actor_def *actor, int direction);
int map_move_actor(struct map_def *map, struct actor_def *actor, int new_x, int new_y);
void map_find_space(const struct map_def *map, int *x, int *y);
void map_tick(struct map_def *map, struct actor_def *player);
int map_was_seen(const struct map_def *map, int x, int y);
int map_in_view(const struct map_def *map, int x, int y);
void map_do_fov(struct map_def *map, int from_x, int from_y, int range);
void map_dump(const struct map_def *map, const char *filename);
struct room_def* map_get_room(struct map_def *map, int x, int y);

struct map_def* map_generate(struct dungeon_def *dungeon);

void rng_init();
void rng_init_time();
int rng_max(int max);
int rng_range(int min, int max);
int rng_roll(int count, int sides);

int yes_or_no(const char *prompt, const char *line2, int default_answer);
void message_box(const char *line1, const char *line2);

void town_loop();
void delve_loop(struct dungeon_def *dungeon);


extern int wants_to_quit;
extern const char *size_names[SIZE_COUNT];
extern const char *goal_names[GOAL_COUNT];

#endif
