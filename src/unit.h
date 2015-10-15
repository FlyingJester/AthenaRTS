#pragma once
#include "spriteset.h"

/* For an equal attack and defense, this is the number of attacks it takes to defeat a unit. */
#define ATTACK_CONSTANT 4.0f

struct Athena_Unit;
struct Athena_Field;

typedef int (*Athena_Ability)(struct Athena_Unit *that, struct Athena_Field *field);

struct Athena_Class {
    float defense, attack;
    unsigned movement, attacks;
    unsigned range;
    unsigned is_building, can_build;
    const char *name;
    struct Athena_Spriteset *spriteset;
};

struct Athena_Unit {
    const struct Athena_Class *clazz;
    unsigned owner;
    float health;
    unsigned x, y, movement, attacks;
};

struct Athena_UnitList {
    struct Athena_Unit unit;
    struct Athena_UnitList *next;
};

void Athena_LoadSpriteset(struct Athena_Class *class);

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other);
struct Athena_Unit *Athena_UnitsCollide(struct Athena_UnitList *list, const unsigned x, const unsigned y);
unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b);
void Athena_RenewUnit(struct Athena_Unit *unit);
