#include <stdio.h>
#include <stdlib.h>

#include "darkness.h"

// int ident;
// const char *name;
// const struct attack_def *attacks[MAX_CLASS_ATTACKS];
// int base_stats[STAT_COUNT];
// const char *description;
// STATS:  HP ACC DODG DAM_MIN DAM_MAX PROT SPD CRIT

const struct class_def classes[] = {
    { 0,   "soldier", '@', CP_GREEN, { 0 }, { 100, 75, 5,  15, 25, 10, 5, 15 }, "A warrior trained in the service of armies." },
    { 1,   "medic",   '@', CP_GREEN, { 0 }, { 90,  65, 10, 10, 15, 0,  5, 10 }, "A battlefield medic, trained during the last war." },
    { 100, "goblin",  'g', CP_RED,   { 0 }, { 50,  55, 10, 8,  16, 0,  5, 5  }, "A short, malicious creature related to orcs." },
    { 101, "orc",     'o', CP_RED,   { 0 }, { 70,  65, 5,  12, 20, 50, 5, 10 }, "A powerful, bestial warrior." },
    { -1, NULL },
};

const struct class_def* class_get(int ident) {
    int i = 0;
    while (classes[i].name != NULL) {
        if (classes[i].ident == ident) {
            return &classes[i];
        }
        ++i;
    }
    return NULL;
}

void actor_generic_ai(struct map_def *map, struct actor_def *actor) {
    ++actor->tick;
    if (point_next_to(actor->x, actor->y, map->player->x, map->player->y)) {
        actor_do_attack(map, actor, map->player, NULL);
        actor->tick += TICK_ATTACK;
    } else if (map_in_view(map, actor->x, actor->y)) {
        int dir = direction_between(actor->x, actor->y, map->player->x, map->player->y);
        if (dir != DIR_NONE) {
            actor_action_step(map, actor, dir);
        }
    } else {
        actor_action_step(map, actor, rng_max(DIR_COUNT));
    }
}


struct actor_def* actor_new(int class_ident) {
    struct actor_def *actor = calloc(1, sizeof(struct actor_def));
    if (!actor) return NULL;

    actor->class_ident = class_ident;
    actor->my_class = class_get(class_ident);
    if (!actor->my_class) {
        free(actor);
        return NULL;
    }

    actor->hp = actor->my_class->base_stats[STAT_MAXHP];

    return actor;
}

void actor_destroy(struct actor_def *actor) {
    free(actor);
}

int actor_action_step(struct map_def *map, struct actor_def *actor, int direction) {
    int nx = actor->x;
    int ny = actor->y;
    shift_point(&nx, &ny, direction);

    const struct tile_type *tile = tile_get_info(map_get_tile(map, nx, ny));
    if (!tile || tile->flags & TILE_SOLID) {
        return 0;
    }

    if (map_get_actor(map, nx, ny) != NULL) {
        return 0;
    }

    map_shift_actor(map, actor, direction);
    actor->tick += TICK_MOVE;
    return 1;
}

int actor_get_stat(const struct actor_def *actor, int stat_number) {
    if (actor == NULL)  return -1;

    if (stat_number == STAT_HP) {
        return actor->hp;
    } else {
        return actor->my_class->base_stats[stat_number];
    }
}

const char* actor_get_name(const struct actor_def *actor) {
    if (actor->name[0] != 0) {
        return actor->name;
    } else {
        return actor->my_class->name;
    }
}

void actor_die(struct map_def *map, struct actor_def *actor) {
    map_set_actor(map, actor->x, actor->y, NULL);
    actor->x = actor->y = -1;
    message_format(map, "%s: dies.", actor_get_name(actor));
    if (actor != map->player) {
        actor_destroy(actor);
    }
}

void actor_take_damage(struct map_def *map, struct actor_def *actor, int amount) {
    if (amount <= 0) {
        actor->hp -= amount;
        int max_hp = actor_get_stat(actor, STAT_MAXHP);
        if (actor->hp > max_hp) {
            actor->hp = max_hp;
        }
    } else {
        int prot = actor_get_stat(actor, STAT_PROTECTION);
        if (prot > 0) {
            int prot_amnt = amount * prot / 100;
            amount -= prot_amnt;
        }
        actor->hp -= amount;
        if (actor->hp <= 0) {
            actor_die(map, actor);
        }
    }
}

int actor_do_attack(struct map_def *map, struct actor_def *attacker, struct actor_def *victim, struct attack_def *attack) {
    int hit_chance = actor_get_stat(attacker, STAT_ACCURACY) - actor_get_stat(victim, STAT_DODGE);
    if (attack) {
        hit_chance += attack->stat_mods[STAT_ACCURACY];
    }

    int roll = rng_max(100);
    message_format(map, "[atk roll: %d vs %d]", roll, hit_chance);
    if (roll < hit_chance) {
        int damage_range = actor_get_stat(attacker, STAT_DAMAGE_MAX) - actor_get_stat(attacker, STAT_DAMAGE_MIN);
        int damage = actor_get_stat(attacker, STAT_DAMAGE_MIN);
        if (damage_range > 0) {
            damage += rng_max(damage_range);
        }
        message_format(map, "%s: attacks %s; %d damage.", actor_get_name(attacker), actor_get_name(victim), damage);
        actor_take_damage(map, victim, damage);
    } else {
        message_format(map, "%s: misses %s.", actor_get_name(attacker), actor_get_name(victim));
    }
    return 1;
}

int actor_add_item(struct actor_def *actor, struct item_def *new_item) {
    int first_empty = -1;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (actor->inventory[i]) {
            struct item_def *old_item = actor->inventory[i];
            if (old_item->type_id == new_item->type_id) {
                int max_remaining = old_item->my_type->max_stack - old_item->qty;
                if (max_remaining > 0) {
                    if (max_remaining >= new_item->qty) {
                        old_item->qty += new_item->qty;
                        item_destroy(new_item);
                        return 1;
                    } else {
                        new_item->qty -= max_remaining;
                        old_item->qty += max_remaining;
                    }
                }
            }
        } else if (first_empty == -1) {
            first_empty = i;
        }
    }

    if (first_empty < 0) {
        return 0;
    }
    actor->inventory[first_empty] = new_item;
    return 1;
}

int actor_item_qty(struct actor_def *actor, int item_type) {
    int qty_found = 0;

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (actor->inventory[i] && actor->inventory[i]->type_id == item_type) {
            qty_found += actor->inventory[i]->qty;
        }
    }

    return qty_found;
}

int actor_remove_items(struct actor_def *actor, int item_type, int to_remove) {
    if (actor_item_qty(actor, item_type) < to_remove) {
        return 0;
    }

    for (int i = 0; i < INVENTORY_SIZE && to_remove > 0; ++i) {
        if (actor->inventory[i]) {
            if (actor->inventory[i]->type_id == item_type) {
                if (actor->inventory[i]->qty > to_remove) {
                    actor->inventory[i]->qty -= to_remove;
                    return 1;
                } else {
                    to_remove -= actor->inventory[i]->qty;
                    item_destroy(actor->inventory[i]);
                    actor->inventory[i] = NULL;
                }
            }
        }
    }
    return 1;
}